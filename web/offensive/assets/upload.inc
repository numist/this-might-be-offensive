<?

require_once("offensive/assets/id3.inc");

// TODO: move all calls to uploadForm() out, indicate with return values.

global $upload_display_prompt, $tmpdir, $filetypes;
$upload_display_prompt = true;
$tmpdir = "quarantine";
$filetypes = array("jpg", "jpeg", "gif", "png");


/*
 * uploading_file -- determines whether or not a file is being uploaded
 * Note: does not detect upload-in-progress
 *
 * Side effects: none.
 *
 * Result: boolean, is there a file to attempt uploading.
 */
function uploading_file() {
	if(array_key_exists("image", $_FILES) &&
		is_array($_FILES['image']) &&
		array_key_exists("name", $_FILES['image']) &&
		$_FILES['image']['name'] != "" &&
		array_key_exists("size", $_FILES['image']) &&
		$_FILES['image']['size'] > 0 &&
		!exceededUploadLimit($_SESSION['userid'])) {

		return true;
	}
	return false;
}

function getHash( $filePath ) {
	$file = fopen( $filePath, "r" );
	$filedata = fread ( $file, min( 4096, filesize( $filePath ) ) );
	fclose( $file );
	return md5( $filedata );
}

function exceededUploadLimit($uid) {
	return (numUploadsRemaining( $uid ) === 0 && $_SESSION['status'] != 'admin');
}

function numUploadsRemaining( $uid ) {
	$limit = numAllowedUploads( $_SESSION['userid'] );
	$used = numUploadsToday( $_SESSION['userid'] );
	return ($limit - $used ) > 0 ? ($limit - $used ) : 0;
}

function numUploadsToday( $uid ) {
	$sql = "SELECT count( id ) as thecount FROM offensive_uploads WHERE userid = " . $_SESSION['userid'] . " AND timestamp > DATE_SUB( NOW(), INTERVAL 1 DAY ) AND type='image'";
	$result = tmbo_query( $sql );

	$row = mysql_fetch_assoc( $result );

	return $row[ 'thecount' ];

}

function numAllowedUploads( $uid ) {

	// Fipi Lele is allowed (effectively) unlimited uploads.
	if($uid == 143) {
		return 9999;
	}

	$sql = "SELECT COUNT( vote ) AS thecount, vote
				FROM offensive_comments, offensive_uploads
				WHERE vote
					AND offensive_comments.fileid = offensive_uploads.id
					AND offensive_uploads.userid = $uid
					AND offensive_uploads.timestamp > DATE_SUB( NOW(), INTERVAL 6 MONTH )
				GROUP  BY vote";

	$result = tmbo_query( $sql );
	$good = 0;
	$bad = 0;

	while( $row = mysql_fetch_assoc( $result ) ) {
		switch( $row['vote'] ) {
			case "this is good":
				$good = $row['thecount'];
			break;

			case "this is bad":
				$bad = $row['thecount'];
			break;
		}
	}

	return min( ($bad > 0) ? round( 1 + ($good/$bad) * 2 ) : round(1 + ($good/20) * 2), 40 );

}

function requestDetail() {
	// XXX: would be nice to get some user data in here at some point.
	ob_start();
	var_dump( $_SERVER );
	var_dump( $_REQUEST );
	if(array_key_exists("postdetail", $_SESSION))
		var_dump( unserialize($_SESSION['postdetail']) );
	$string = ob_get_contents();
	ob_end_clean();
	return $string;
}

function getTransform($orientation) {
	/*
	 *   1        2       3      4         5            6           7          8
     *
	 * 888888  888888      88  88      8888888888  88                  88  8888888888
	 * 88          88      88  88      88  88      88  88          88  88      88  88
	 * 8888      8888    8888  8888    88          8888888888  8888888888          88
	 * 88          88      88  88
	 * 88          88  888888  888888
	 */

	switch($orientation) {
		case 2:
			return "-flop";
			break;
		case 3:
			return "-rotate 180";
			break;
		case 4:
			return "-flip";
			break;
		case 5:
			return "-transpose";
			break;
		case 6:
			return "-rotate 90";
			break;
		case 7:
			return "-transverse";
			break;
		case 8:
			return "-rotate 270";
			break;
		default:
			return "";
	}
}

function killUpload($postdata) {
	global $tmpdir;
	unset($_SESSION['postdata']);
	if(file_exists($tmpdir."/".$postdata['tmpname']))
		unlink($tmpdir."/".$postdata['tmpname']);
}

###############################################################################
/* upload -- upload a file.
 * this function sets up/tears down upload state from REQUEST data, and
 * determines the type of upload, handing it off to the appropriate subdriver.
 */
function upload() {
	global $tmpdir;

	// if we are mid-upload, restore the right datas and continue
	if(array_key_exists('postdata', $_SESSION)) {
		$postdata = unserialize($_SESSION['postdata']);

		// maybe cancel the upload
		if(array_key_exists('killit', $_POST)) {
			$wasavatar = array_key_exists("avatar", $postdata);
			box("another one bites the dust", "all right!");
			killUpload($postdata);
			uploadForm(false, $wasavatar);
			return;
		}

		// make sure the postdata gets populated correctly
		$flags = array("rotate", "norotate",
		               "resize", "noresize",
		               "repost",
		               "resample", "noresample");
		foreach($flags as $flag) {
			if(array_key_exists($flag, $_POST)) {
				$postdata[$flag] = $_POST[$flag];
			}
		}

		// if the file doesn't exist, we're not doing any posting, are we...
		if(!file_exists($tmpdir."/".$postdata['tmpname'])) {
			trigger_error("post from SESSION failed: file not found.", E_USER_WARNING);
			$message = "couldn't find the file you wanted to upload, sorry.  try again?";
			box($message);
			killUpload($postdata);
			unset($postdata);
			unset($_SESSION['postdata']);
			uploadForm();
			return;
		}
	} else if(!uploading_file() && array_key_exists("submit", $_POST) && array_key_exists("topic", $_POST)) {
		return upload_topic();
	} else if(!uploading_file()) {
		uploadForm();
		return null;
	} else {
		// move the file to its new home and set up some state.
		$postdata = array();
		$postdata['tmpname'] = explode("/", $_FILES['image']['tmp_name']);
		$postdata['tmpname'] = array_pop($postdata['tmpname']);
		// having the file extension helps ImageMagick do its thing.
		$postdata['file_extension'] = getFileExtension($_FILES['image']['name']);
		$postdata['tmpname'] = $postdata['tmpname'].$postdata['file_extension'];
		$postdata['filename'] = $_FILES['image']['name'];
		if(array_key_exists('nsfw', $_POST))
			$postdata['nsfw'] = $_POST['nsfw'];
		if(array_key_exists('tmbo', $_POST))
			$postdata['tmbo'] = $_POST['tmbo'];

		ensureDirExists($tmpdir);
		move_uploaded_file($_FILES['image']['tmp_name'], $tmpdir."/".$postdata['tmpname']) or trigger_error("", E_USER_ERROR);
		chmod($tmpdir."/".$postdata['tmpname'], 0644);

		/*
		 * prevent filenames ending in .php from getting through--the webserver will try executing them as scripts and there is a
		 * potential for a security compromise if someone comes up with a polyglot that is valid PHP and manages to fool our
		 * filetype detection into thinking that it's a valid image.
		 */
		if($postdata['file_extension'] == '.php') {
			killUpload($postdata);
			unset($postdata);
			unset($_SESSION['postdata']);
			$message = "sorry, filenames can't end in .php";
			box($message);
			uploadForm();
			return;
		}
	}

	switch(upload_type($postdata)) {
		case "image":
			return upload_image($postdata);
		case "avatar":
			return upload_avatar($postdata);
		case "audio":
			return upload_audio($postdata);
	}

	global $filetypes;

	box("acceptable file types: ".implode(", ", $filetypes).".", "sorry.");

	mail( "thismightbe@numist.net",
	      "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'] .
	      $postdata['tmpname'], requestDetail(),
	      "From: offensive@thismight.be (this might be offensive)\r\n" );

	uploadForm();
	return false;
}

###############################################################################
// memoized.
function upload_type(&$postdata) {
	// cache type so we don't whack at the disk too much.
	global $upload_identified_type, $tmpdir, $upload_identified_key;
	if($upload_identified_type != "") {
		return $upload_identified_type;
	}

	if(is_array($postdata)) {
		$filename = $tmpdir."/".$postdata['tmpname'];
	} else {
		$filename = $postdata;
	}

	$format_info = array(

		// Audio formats

		// AC-3   - audio      - Dolby AC-3 / Dolby Digital
		'ac3'  => array(
					'pattern'   => '^\x0B\x77',
					'group'     => 'audio-forbidden',
				  ),

		// AAC  - audio       - Advanced Audio Coding (AAC) - ADIF format
		'adif' => array(
					'pattern'   => '^ADIF',
					'group'     => 'audio-forbidden',
				  ),


		// AAC  - audio       - Advanced Audio Coding (AAC) - ADTS format (very similar to MP3)
		'adts' => array(
					'pattern'   => '^\xFF[\xF0-\xF1\xF8-\xF9]',
					'group'     => 'audio-forbidden',
				  ),


		// AU   - audio       - NeXT/Sun AUdio (AU)
		'au'   => array(
					'pattern'   => '^\.snd',
					'group'     => 'audio-forbidden',
				  ),

		// AVR  - audio       - Audio Visual Research
		'avr'  => array(
					'pattern'   => '^2BIT',
					'group'     => 'audio-forbidden',
				  ),

		// BONK - audio       - Bonk v0.9+
		'bonk' => array(
					'pattern'   => '^\x00(BONK|INFO|META| ID3)',
					'group'     => 'audio-forbidden',
				  ),

		// FLAC - audio       - Free Lossless Audio Codec
		'flac' => array(
					'pattern'   => '^fLaC',
					'group'     => 'audio-forbidden',
				  ),

		// LA   - audio       - Lossless Audio (LA)
		'la'   => array(
					'pattern'   => '^LA0[2-4]',
					'group'     => 'audio-forbidden',
				  ),

		// LPAC - audio       - Lossless Predictive Audio Compression (LPAC)
		'lpac' => array(
					'pattern'   => '^LPAC',
					'group'     => 'audio-forbidden',
				  ),

		// MIDI - audio       - MIDI (Musical Instrument Digital Interface)
		'midi' => array(
					'pattern'   => '^MThd',
					'group'     => 'audio-forbidden',
				  ),

		// MAC  - audio       - Monkey's Audio Compressor
		'mac'  => array(
					'pattern'   => '^MAC ',
					'group'     => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (assorted sub-formats)
		'mod'  => array(
					'pattern'   => '^.{1080}(M.K.|[5-9]CHN|[1-3][0-9]CH)',
					'group'     => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (Impulse Tracker)
		'it'   => array(
					'pattern'   => '^IMPM',
					'group'     => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (eXtended Module, various sub-formats)
		'xm'   => array(
					'pattern'   => '^Extended Module',
					'group'     => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (ScreamTracker)
		's3m'  => array(
					'pattern'   => '^.{44}SCRM',
					'group'     => 'audio-forbidden',
				  ),

		// MPC  - audio       - Musepack / MPEGplus
		'mpc'  => array(
					'pattern'   => '^(MP\+|[\x00\x01\x10\x11\x40\x41\x50\x51\x80\x81\x90\x91\xC0\xC1\xD0\xD1][\x20-37][\x00\x20\x40\x60\x80\xA0\xC0\xE0])',
					'group'     => 'audio-forbidden',
				  ),

		// MP3  - audio       - MPEG-audio Layer 3 (very similar to AAC-ADTS)
		'mp3'  => array(
					'pattern'   => '^\xFF[\xE2-\xE7\xF2-\xF7\xFA-\xFF][\x00-\xEB]',
					'group'     => 'audio',
				  ),

		// OFR  - audio       - OptimFROG
		'ofr'  => array(
					'pattern'   => '^(\*RIFF|OFR)',
					'group'     => 'audio-forbidden',
				  ),

		// RKAU - audio       - RKive AUdio compressor
		'rkau' => array(
					'pattern'   => '^RKA',
					'group'     => 'audio-forbidden',
				  ),

		// SHN  - audio       - Shorten
		'shn'  => array(
					'pattern'   => '^ajkg',
					'group'     => 'audio-forbidden',
				  ),

		// TTA  - audio       - TTA Lossless Audio Compressor (http://tta.corecodec.org)
		'tta'  => array(
					'pattern'   => '^TTA',  // could also be '^TTA(\x01|\x02|\x03|2|1)'
					'group'     => 'audio-forbidden',
				  ),

		// VOC  - audio       - Creative Voice (VOC)
		'voc'  => array(
					'pattern'   => '^Creative Voice File',
					'group'     => 'audio-forbidden',
				  ),

		// VQF  - audio       - transform-domain weighted interleave Vector Quantization Format (VQF)
		'vqf'  => array(
					'pattern'   => '^TWIN',
					'group'     => 'audio-forbidden',
				  ),

		// WV  - audio        - WavPack (v4.0+)
		'wv'   => array(
					'pattern'   => '^wvpk',
					'group'     => 'audio-forbidden',
				  ),


		// Audio-Video formats

		// ASF  - audio/video - Advanced Streaming Format, Windows Media Video, Windows Media Audio
		'asf'  => array(
					'pattern'   => '^\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C',
					'group'     => 'audio-video-forbidden',
				  ),

		// BINK  - audio/video - Bink / Smacker
		'bink' => array(
					'pattern'   => '^(BIK|SMK)',
					'group'     => 'audio-video-forbidden',
				  ),

		// MKAV - audio/video - Mastroka
		'matroska' => array(
					'pattern'   => '^\x1A\x45\xDF\xA3',
					'group'     => 'audio-video-forbidden',
				  ),

		// MPEG - audio/video - MPEG (Moving Pictures Experts Group)
		'mpeg' => array(
					'pattern'   => '^\x00\x00\x01(\xBA|\xB3)',
					'group'     => 'audio-video-forbidden',
				  ),

		// NSV  - audio/video - Nullsoft Streaming Video (NSV)
		'nsv'  => array(
					'pattern'   => '^NSV[sf]',
					'group'     => 'audio-video-forbidden',
				  ),

		// Ogg  - audio/video - Ogg (Ogg-Vorbis, Ogg-FLAC, Speex, Ogg-Theora(*), Ogg-Tarkin(*))
		'ogg'  => array(
					'pattern'   => '^OggS',
					'group'     => 'audio-forbidden',
				  ),

		// QT   - audio/video - Quicktime
		'quicktime' => array(
					'pattern'   => '^.{4}(cmov|free|ftyp|mdat|moov|pnot|skip|wide)',
					'group'     => 'audio-video-forbidden',
				  ),

		// RIFF - audio/video - Resource Interchange File Format (RIFF) / WAV / AVI / CD-audio / SDSS = renamed variant used by SmartSound QuickTracks (www.smartsound.com) / FORM = Audio Interchange File Format (AIFF)
		'riff' => array(
					'pattern'   => '^(RIFF|SDSS|FORM)',
					'group'     => 'audio-video-forbidden',
				  ),

		// Real - audio/video - RealAudio, RealVideo
		'real' => array(
					'pattern'   => '^(\.RMF|.ra)',
					'group'     => 'audio-video-forbidden',
				  ),

		// SWF - audio/video - ShockWave Flash
		'swf' => array(
					'pattern'   => '^(F|C)WS',
					'group'     => 'audio-video-forbidden',
				  ),


		// Still-Image formats

		// BMP  - still image - Bitmap (Windows, OS/2; uncompressed, RLE8, RLE4)
		'bmp'  => array(
					'pattern'   => '^BM',
					'group'     => 'image-forbidden',
				  ),

		// GIF  - still image - Graphics Interchange Format
		'gif'  => array(
					'pattern'   => '^GIF',
					'group'     => 'image',
				  ),

		// JPEG - still image - Joint Photographic Experts Group (JPEG)
		'jpg'  => array(
					'pattern'   => '^\xFF\xD8\xFF',
					'group'     => 'image',
				  ),

		// PCD  - still image - Kodak Photo CD
		'pcd'  => array(
					'pattern'   => '^.{2048}PCD_IPI\x00',
					'group'     => 'image-forbidden',
				  ),


		// PNG  - still image - Portable Network Graphics (PNG)
		'png'  => array(
					'pattern'   => '^\x89\x50\x4E\x47\x0D\x0A\x1A\x0A',
					'group'     => 'image',
				  ),


		// TIFF  - still image - Tagged Information File Format (TIFF)
		'tiff' => array(
					'pattern'   => '^(II\x2A\x00|MM\x00\x2A)',
					'group'     => 'image-forbidden',
				  ),


		// Data formats

		// ISO  - data        - International Standards Organization (ISO) CD-ROM Image
		'iso'  => array(
					'pattern'   => '^.{32769}CD001',
					'group'     => 'misc-forbidden',
				  ),

		// RAR  - data        - RAR compressed data
		'rar'  => array(
					'pattern'   => '^Rar\!',
					'group'     => 'archive-forbidden',
				  ),

		// SZIP - audio       - SZIP compressed data
		'szip' => array(
					'pattern'   => '^SZ\x0A\x04',
					'group'     => 'archive-forbidden',
				  ),

		// TAR  - data        - TAR compressed data
		'tar'  => array(
					'pattern'   => '^.{100}[0-9\x20]{7}\x00[0-9\x20]{7}\x00[0-9\x20]{7}\x00[0-9\x20\x00]{12}[0-9\x20\x00]{12}',
					'group'     => 'archive-forbidden',
				  ),

		// GZIP  - data        - GZIP compressed data
		'gz'  => array(
					'pattern'   => '^\x1F\x8B\x08',
					'group'     => 'archive-forbidden',
				  ),

		// ZIP  - data        - ZIP compressed data
		'zip'  => array(
					'pattern'   => '^PK\x03\x04',
					'group'     => 'archive-forbidden',
				  )
	);

	// check for an ID3 header, we skip over it (if it exists) when detecting filetype.
	$fp = fopen($filename, 'r');
	$dataoffset = 0;
	fseek($fp, 0, SEEK_SET);
	$header = fread($fp, 10);
	if (substr($header, 0, 3) == 'ID3') {
		// Big-Endian conversion to the data offset
		$dataoffset = 0;
		$byteword = substr($header, 6, 4);
		$bytewordlen = strlen($byteword);
		for ($i = 0; $i < $bytewordlen; $i++) {
				$intvalue = $intvalue | (ord($byteword{$i}) & 0x7F) << (($bytewordlen - 1 - $i) * 7);
		}
		$dataoffset += 10; // length of ID3v2 tag in 10-byte header doesn't include 10-byte header length
	}

	// read 32 kb file data
	fseek($fp, $dataoffset, SEEK_SET);
	$filedata = fread($fp, 32774);

	// Identify file format - loop through $format_info and detect with reg expr
	foreach ($format_info as $format_name => $info) {
		// Using preg_match() instead of ereg() - much faster
		// The /s switch on preg_match() forces preg_match() NOT to treat
		// newline (0x0A) characters as special chars but do a binary match
		if (preg_match('/'.$info['pattern'].'/s', $filedata)) {
			$postdata["format_name"] = $format_name;
			if($info['group'] == "image" && (array_key_exists('avatar', $_POST) ||
			                                 array_key_exists('avatar', $postdata))) {
					$postdata['avatar'] = true;
					$info['group'] = "avatar";
			} else if(array_key_exists('avatar', $_POST)) {
				box("avatars must be image files", "hard to look at anything else");
				return "";
			}
			$upload_identified_key = $format_name;
			$upload_identified_type = $info['group'];
			return $upload_identified_type;
		}
	}

	$foo = new getid3_id3v2($fp, $inf);
	if(count($inf) > 0 && preg_match('/\.mp3$/i', $filename)) {
		$upload_identified_key = "mp3";
		$upload_identified_type = "audio";
		return $upload_identified_type;
	}

	return "";
}

###############################################################################
function upload_image($postdata) {
	if(upload_largerthan($postdata, 4*1024*1024)) return false;
	if(upload_checkrepost($postdata)) return false;
	if(upload_checkrotation($postdata)) return false;
	if(upload_checkdims($postdata)) return false;
	if(upload_checksize($postdata)) return false;
	return upload_insert_file($postdata);
}

function upload_avatar($postdata) {
	if(upload_largerthan($postdata, 4*1024*1024)) return false;
	if(upload_checkrotation($postdata)) return false;
	if(upload_checkdims($postdata)) return false;
	if(upload_checksize($postdata)) return false;
	return upload_insert_file($postdata);
}

function upload_topic() {
	return upload_insert_topic();
}

function upload_audio($postdata) {
	global $tmpdir;

	if(upload_largerthan($postdata, 10*1024*1024)) return false;
	if(upload_checkrepost($postdata)) return false;

	return upload_insert_file($postdata);
}

###############################################################################
// does the upload have reasonable dimensions? (pictures only)
function upload_checkdims($postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic" || upload_type($postdata) == "audio")
		trigger_error("can't check pixel dimensions of non-image files", E_USER_ERROR);

	if(array_key_exists("noresize", $postdata))
		return false;

	if (array_key_exists("resize", $postdata)) {
		$cmd = "convert -resize 1024x1024 $tmpdir/".$postdata['tmpname']." $tmpdir/".$postdata['tmpname'];
		$err = shell_exec($cmd);
		if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);

		$postdata['noresize'] = $postdata['resize'];
		unset($postdata['resize']);
		return false;
	}

	$filesize = filesize($tmpdir."/".$postdata['tmpname']);
	list($width, $height, $type, $attr) = getimagesize($tmpdir."/".$postdata['tmpname']);

	// sanity check
	if(!is_numeric( $width ) || !is_numeric($height) || $width * $height == 0 ) {
		trigger_error("can't check pixel dimensions of non-image files", E_USER_ERROR);
	}

	// outright reject anything of the spaceball.gif variety
	if($width < 5 && $height < 5) {
		$message = "Why would you try to upload a $width x $height pixel image?";
		box($message, "flickr thief!");
		killUpload($postdata);
		uploadForm();
		return true;
	}

	if($height <= 1024 || $width <= 1024) {
		return false;
	}

	$message = "This image is rather large ($width x $height).<br />If you want, I can try resizing it for you.<br/><br/>
	<form action=\"{$_SERVER['PHP_SELF']}\" method=\"POST\">
		<input type=\"hidden\" name=\"c\" value=\"upload\">
		<input type=\"submit\" name=\"killit\" value=\"Cancel this upload.\"/>
		<input type=\"submit\" name=\"resize\" value=\"Resize it for me.\"/>
		<input type=\"submit\" name=\"noresize\" value=\"Post it anyway.\"/>
	</form>";
	box($message, "is it 'post your desktop' day?");
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

// does the upload have a reasonable bits per pixel? (pictures only)
function upload_checksize(&$postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic" || upload_type($postdata) == "audio")
		trigger_error("can't check bytes per pixel of non-image files", E_USER_ERROR);

	if(array_key_exists("noresample", $postdata))
		return false;

	if(array_key_exists("resample", $postdata)) {
		// convert from file.ext to file.jpg, quality 90
		$newtmp = substr($postdata['tmpname'], 0, 0 - strlen($postdata['file_extension'])).".jpg";
		$newfilename = substr($postdata['filename'], 0, 0 - strlen($postdata['file_extension'])).".jpg";

		$cmd = "convert -quality 90 $tmpdir/".$postdata['tmpname']." $tmpdir/$newtmp";
		$err = shell_exec($cmd);
		if(strlen($err) > 0)
			trigger_error("ImageMagick failed with: $err", E_USER_ERROR);

		// clean up the (potentially) extra file
		if($newfilename != $postdata['filename'])
			unlink("$tmpdir/{$postdata['tmpname']}");

		$postdata['tmpname'] = $newtmp;
		$postdata['filename'] = $newfilename;
		$postdata['file_extension'] = ".jpg";
		$postdata['format'] = "jpg";
		$postdata['format_name'] = "jpg";

		$postdata['noresample'] = $postdata['resample'];
		unset($postdata['resample']);

		return false;
	}

	// this stuff is for extra large (uncompressed) images.  not sure if we care as much.
	$filesize = filesize($tmpdir."/".$postdata['tmpname']);

	list($width, $height, $type, $attr) = getimagesize($tmpdir."/".$postdata['tmpname']);

	$bytesPerPixel = 0.0;
	$expectedFileSize = round(($width * $height * 0.2)/1024);
	$bytesPerPixel = ( 1.0 * $filesize / (1.0 * $width * $height ) );
	$filesize = round($filesize/1024);

	if($bytesPerPixel <= 0.45)
		return false;

	$message = "This image has a pretty big filesize for its dimensions. Expected file size: ${expectedFileSize}k. Your file: ${filesize}k.<br />If you want, I can resample it as an 90% quality JPG image.<br /><br />
		<form action=\"{$_SERVER['PHP_SELF']}\" method=\"POST\">
			<input type=\"hidden\" name=\"c\" value=\"upload\">
			<input type=\"submit\" name=\"killit\" value=\"Cancel this upload.\"/>
			<input type=\"submit\" name=\"resample\" value=\"Compress my file.\"/>
			<input type=\"submit\" name=\"noresample\" value=\"Post it anyway.\"/>
		</form>";
	if($postdata['format'] == 'gif') $message .= "<br /><br />(If this is an animated gif, don't click the \"Compress\" button!)";

	box($message, "whoa, nelly");
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

// has this upload been posted before? (images and audio only)
function upload_checkrepost(&$postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic")
		trigger_error("can't compute hash of non-files", E_USER_ERROR);

	if(array_key_exists("repost", $postdata))
		return false;

	$postdata['hash'] = getHash($tmpdir."/".$postdata['tmpname']);

	if(strlen($postdata['hash']) < 15) trigger_error("bad hash?", E_USER_ERROR);

	$sql = "SELECT *
			FROM offensive_uploads
			WHERE hash = '".$postdata['hash']."' AND type = '".upload_type($postdata)."'
			ORDER by timestamp DESC";
	$result = tmbo_query( $sql );
	if( mysql_num_rows( $result ) == 0 ) return false;

	$filePath=dirname(realpath($_SERVER['SCRIPT_FILENAME']));

	?><div class="heading">poster child:</div>
	<div class="bluebox">
	<div style="text-align:center">
	Your file appears to be a repost of:
	<br /><center>
	<table><?
	while($row = mysql_fetch_assoc($result)) {
		$upload = new Upload($row);
		$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";

		?><tr class="<?= $css ?>"><?
			if(upload_type($postdata) == "image" || upload_type($postdata) == "avatar") { ?>
				<td height="100px" width="100px">

					<a href="pages/pic.php?id=<?= $upload->id() ?>"
						<? if($upload->filtered()) { ?>
							onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
						<? } ?>
						><img name="th<?= $upload->id()?>"
							src="<?= $upload->filtered()
							? "/offensive/graphics/th-filtered.gif" 
							: $upload->thumbURL() ?>"
					/></a>

				</td>
			<? } ?>
		<td><a href="/offensive/?c=comments&fileid=<?= $upload->id(); ?>" target="_blank"><?= htmlEscape($upload->filename()) ?></a></td>
		</tr><?
	}
	?></table></center><br />
	<form action="<?= $_SERVER['PHP_SELF'] ?>" method="POST">
		<input type="hidden" name="c" value="upload">
		<input type="submit" name="killit" value="Cancel this upload."/>
		<input type="submit" name="repost" value="Post it anyway."/>
	</form>
	</div>
	</div>
	<?
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

function upload_checkrotation($postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic" || upload_type($postdata) == "audio")
		trigger_error("can't check rotation of non-image files", E_USER_ERROR);

	if(array_key_exists("rotate", $postdata) && function_exists("exif_read_data")) {
		$filename = $tmpdir."/".$postdata['tmpname'];
		$exif = @exif_read_data($tmpdir."/".$postdata['tmpname']);

		if($exif && array_key_exists("Orientation", $exif)) {
			$transform = getTransform($exif['Orientation']);
			if($transform != "") {
				// convert.
				$cmd = "convert $transform $tmpdir/{$postdata['tmpname']} $tmpdir/{$postdata['tmpname']}";
				$err = shell_exec($cmd);
				if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);
			}
		}
		// avoid re-rotating (or attempting to) again
		$postdata['norotate'] = $postdata['rotate'];
		unset($postdata['rotate']);
	}

	if(array_key_exists("norotate", $postdata) ||
	   array_key_exists("rotate", $postdata)) {
		if(file_exists("$tmpdir/th{$postdata['tmpname']}"))
			unlink("$tmpdir/th{$postdata['tmpname']}");
		if(file_exists("$tmpdir/rh{$postdata['tmpname']}"))
			unlink("$tmpdir/rh{$postdata['tmpname']}");
	}

	if(array_key_exists("norotate", $postdata))
		return false;

	$filename = $tmpdir."/".$postdata['tmpname'];
	$exif = @exif_read_data($filename);

	if(!$exif || !array_key_exists("Orientation", $exif)) {
		return false;
	}

	$transform = getTransform($exif['Orientation']);

	if($transform == "")
		return false;

	$cmd = "convert -resize 100x100 $tmpdir/{$postdata['tmpname']} $tmpdir/th{$postdata['tmpname']}";
	$err = shell_exec($cmd);
	if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);

	$cmd = "convert $transform -resize 100x100 $tmpdir/{$postdata['tmpname']} $tmpdir/rh{$postdata['tmpname']}";
	$err = shell_exec($cmd);
	if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);

	$message = "Is it sideways?<br />If you want, I can try rotating it for you.<br/><br/>
	<center><table><tr><td align='middle'>Original</td><td align='middle'>Fixed</td></tr>
	<tr><td><img src='/offensive/$tmpdir/th{$postdata['tmpname']}'></td><td><img src='/offensive/$tmpdir/rh{$postdata['tmpname']}'></td></tr></table></center><br/>

	<form action=\"{$_SERVER['PHP_SELF']}\" method=\"POST\">
		<input type=\"hidden\" name=\"c\" value=\"upload\">
		<input type=\"submit\" name=\"killit\" value=\"Cancel this upload.\"/>
		<input type=\"submit\" name=\"rotate\" value=\"Rotate it for me.\"/>
		<input type=\"submit\" name=\"norotate\" value=\"Post it anyway.\"/>
	</form>";
	box($message, "¡sǝou ɥo");
	$_SESSION['postdata'] = serialize($postdata);

	return true;
}

function upload_largerthan($postdata, $bytes) {
	global $tmpdir;

	if(upload_type($postdata) == "topic")
		trigger_error("can't compute size of non-files", E_USER_ERROR);

	$filesize = filesize($tmpdir."/".$postdata['tmpname']);

	if($filesize <= $bytes) return false;

	killUpload($postdata);
	box("that file is too big to upload, try something else", "WHARRGARBL");
	uploadForm();
	return true;
}

###############################################################################
function upload_insert_file($postdata) {
	global $tmpdir;

	// it's possible (it's happened in the past!) for the time in sql to differ from PHP
	// use the SQL server's time as the trusted source
	$sql = "SELECT NOW()";
	$res = tmbo_query($sql);
	list($now) = mysql_fetch_array($res);

	// basePath()
	$timestamp = strtotime($now);
	$year = date( "Y", $timestamp );
	$month = date( "m", $timestamp );
	$day = date( "d", $timestamp );
	$path = get_include_path()."/offensive/uploads/$year/$month/$day/".upload_type($postdata)."/";

	$nsfw = array_key_exists('nsfw', $postdata) ? 1 : 0;
	$tmbo = array_key_exists('tmbo', $postdata) ? 1 : 0;

	if(strlen($postdata['hash']) < 15)
		$postdata['hash'] = getHash($tmpdir."/".$postdata['tmpname']);

	$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw,tmbo,hash,type,status )
			   VALUES ( " . $_SESSION['userid'] . ", '" .
			                sqlEscape( $postdata['filename'] ) . "', '" .
			                $_SERVER['REMOTE_ADDR'] . "',
			                $nsfw, $tmbo, '".$postdata['hash']."', '" .
			                upload_type( $postdata ) . "', 'pending')";
	tmbo_query($sql);
	$fileid = mysql_insert_id();

	ensureDirExists("$path");

	###########################################################################
	// image types get thumbnails
	if(upload_type($postdata) == "image" || upload_type($postdata) == "avatar") {
		ensureDirExists("$path/thumbs");

		// generate the thumbnail

		#$cmd = "convert -resize 100x100 $tmpdir/".$postdata['tmpname']." $tmpdir/th".$postdata['tmpname'];		
		$cmd = "convert -define jpeg:size=200x200 $tmpdir/".$postdata['tmpname']."[0] -thumbnail 100x100^ -gravity center -extent 133x100 $tmpdir/th".$postdata['tmpname'];
		
		$err = shell_exec($cmd);
		if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);

		// move into place
		rename($tmpdir."/th".$postdata['tmpname'],
		     "$path/thumbs/th$fileid".$postdata['file_extension'])
			or trigger_error("could not copy thumbnail to $path/thumbs!", E_USER_ERROR);
	} else if(upload_type($postdata) == "audio") {
		$fp = fopen($tmpdir."/".$postdata['tmpname'], 'r');
		$id3 = new getid3_id3v2($fp, $info);
		ensureDirExists("$path/thumbs");

		// check for a valid id3 tag
		if(array_key_exists('id3v2', $info)) {
			$artdata = false;
			
			// different kinds of embeddable images:
			if(array_key_exists('APIC', $info['id3v2'])
			   && count($info['id3v2']['APIC']) > 0
			   && array_key_exists('data', $info['id3v2']['APIC'][0])) {
				$artdata = $info['id3v2']['APIC'][0]['data'];
				
			} else if(array_key_exists('PIC', $info['id3v2'])
			          && count($info['id3v2']['PIC']) > 0
			          && array_key_exists('data', $info['id3v2']['PIC'][0])) {
				$artdata = $info['id3v2']['PIC'][0]['data'];
			}
			
			if($artdata) {
				$thumbname = "$path/thumbs/th$fileid";
				file_put_contents($thumbname, $artdata);

				// generate the thumbnail if necessary
				list($width, $height, $type, $attr) = getimagesize($thumbname);
				if($width > 100 || $height > 100) {
					$cmd = "convert -resize 100x100 $thumbname $thumbname";
					$err = shell_exec($cmd);
					if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);
				}
			}
		}
	}

	rename($tmpdir."/".$postdata['tmpname'],
	     "$path/".$fileid."_".$postdata['filename'])
		or trigger_error("could not copy file to $path!", E_USER_ERROR);

	// initialize a row in the count cache
	$sql = "INSERT INTO offensive_count_cache ( threadid, good, bad, repost, tmbo, comments )
				VALUES ( $fileid, 0, 0, 0, 0, 0 )";
	tmbo_query($sql);

	// all set, let's go
	$sql = "UPDATE offensive_uploads SET status = 'normal' where id = $fileid";
	tmbo_query($sql);

	$sql = "INSERT INTO offensive_subscriptions (userid, fileid )
	        VALUES ( " . $_SESSION['userid'] . ", $fileid ) ";
	tmbo_query( $sql );

	$message = "<p>
		Thanks. Your upload can be viewed <a href=\"./pages/pic.php?id=$fileid\">here</a>.
	</p>
	<p>
			You may <a href=\"./?c=comments&fileid=$fileid\">comment on this file here</a>.
	</p>
	<p>
		<a href=\"./\">Back to the list</a>
	</p>";
	box($message, "yummy.");
	if(upload_type($postdata) != "avatar")
		uploadForm(true);

	unset($_SESSION['postdata']);
}

function upload_insert_topic() {
	// TODO
}

?>
