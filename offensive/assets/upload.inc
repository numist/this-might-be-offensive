<?php

require_once("offensive/assets/id3.inc");
require_once("offensive/assets/xmlrpc.inc");
require_once("offensive/assets/isk.inc");

// TODO: move all calls to uploadForm() out, indicate with return values.

global $upload_display_prompt, $tmpdir, $filetypes;
$upload_display_prompt = true;
$tmpdir = "quarantine";
$filetypes = array("jpg", "jpeg", "gif", "png", "webp");

// largest image or avatar we'll take in, in bytes. PHP (services/php/php.ini)
// and nginx (client_max_body_size) both allow 20M, so this is the limit that
// bites.
const UPLOAD_MAX_BYTES = 16 * 1024 * 1024;
// largest image we'll post. A still image between this and UPLOAD_MAX_BYTES
// (a 48 MP phone photo, say) has to go through the resize tool first; an
// animated one can't be resized, so it's turned away.
const UPLOAD_MAX_POST_BYTES = 8 * 1024 * 1024;

// images bigger than this (long side in px, total pixels, or bytes) get the
// resize tool. Most phone photos are over the byte limit, on purpose: the tool
// offers a smaller copy, and posting the original is one click.
const UPLOAD_PROMPT_MAX_SIDE = 4096;
const UPLOAD_PROMPT_MAX_PIXELS = 16 * 1000 * 1000;
const UPLOAD_PROMPT_MAX_BYTES = 1024 * 1024;
// the resize tool's starting settings and its bounds. The long edge can't go
// past the original's (no enlarging); quality applies to JPEG and WebP.
const UPLOAD_RESIZE_DEFAULT_SIDE = 2560;
const UPLOAD_RESIZE_MIN_SIDE = 320;
const UPLOAD_RESIZE_DEFAULT_QUALITY = 85;
const UPLOAD_RESIZE_MIN_QUALITY = 10;


/*
 * uploading_file -- determines whether or not a file is being uploaded
 * Note: does not detect upload-in-progress
 *
 * Side effects: none.
 *
 * Result: boolean, is there a file to attempt uploading.
 */
function uploading_file() {
	if(!me()) return false;
	if(TMBO::readonly()) return false;
	if(array_key_exists("image", $_FILES) &&
		is_array($_FILES['image']) &&
		array_key_exists("name", $_FILES['image']) &&
		$_FILES['image']['name'] != "" &&
		array_key_exists("size", $_FILES['image']) &&
		$_FILES['image']['size'] > 0 &&
		!exceededUploadLimit()) {
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

function exceededUploadLimit() {
	if(!me()) return true;
	return (numUploadsRemaining() === 0 && me()->status() != "admin");
}

function numUploadsRemaining() {
	if(!me()) return 0;
	$limit = numAllowedUploads();
	$used = numUploadsToday();
	return ($limit - $used ) > 0 ? ($limit - $used ) : 0;
}

function numUploadsToday() {
	if(!me()) return 0;
	$sql = "SELECT count( id ) as thecount FROM offensive_uploads WHERE userid = ".me()->id()." AND timestamp > DATE_SUB( NOW(), INTERVAL 1 DAY ) AND type='image'";
	$result = tmbo_query( $sql );
	$row = mysqli_fetch_assoc( $result );
	return $row[ 'thecount' ];
}

function needCompression($filename) {
	global $filesize, $expectedFileSize;
	$filesize = filesize($filename);	
	list($width, $height, $type, $attr) = getimagesize($filename);

	$bytesPerPixel = 0.0;
	$expectedFileSize = round(($width * $height * 0.2)/1024);
	$bytesPerPixel = ( 1.0 * $filesize / (1.0 * $width * $height ) );
	if($bytesPerPixel <= 0.75)
		return false;	
	
	return true;
}

function numAllowedUploads() {
	if(!me()) return 0;

	$sql = "SELECT COUNT( vote ) AS thecount, vote
				FROM offensive_comments, offensive_uploads
				WHERE vote
					AND offensive_comments.fileid = offensive_uploads.id
					AND offensive_uploads.userid = ".me()->id()."
					AND offensive_uploads.timestamp > DATE_SUB( NOW(), INTERVAL 6 MONTH )
				GROUP  BY vote";

	$result = tmbo_query( $sql );
	$good = 0;
	$bad = 0;

	while( $row = mysqli_fetch_assoc( $result ) ) {
		switch( $row['vote'] ) {
			case "this is good":
				$good += $row['thecount'];
			break;

			case "this is bad":
				$bad += $row['thecount'];
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

function killUpload($postdata) {
	global $tmpdir;
	unset($_SESSION['postdata']);
	if(file_exists($tmpdir."/".$postdata['tmpname']))
		unlink($tmpdir."/".$postdata['tmpname']);
	upload_resize_discard($postdata);
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
		$flags = array("noresize",
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
		if(array_key_exists("image", $_FILES) &&
		   is_array($_FILES['image']) &&
		   array_key_exists("size", $_FILES['image']) &&
		   $_FILES['image']['size'] == 0) {
			box("that file is way too big. try something else", "boom.");
		}
		
		uploadForm();
		return null;
	} else {
		// move the file to its new home and set up some state.
		$postdata = array();
		$postdata['tmpname'] = explode("/", $_FILES['image']['tmp_name']);
		$postdata['tmpname'] = array_pop($postdata['tmpname']);
		// having the file extension helps ImageMagick do its thing.
		// only a short alphanumeric one, though; it ends up in a path.
		$postdata['file_extension'] = strtolower((string)getFileExtension($_FILES['image']['name']));
		if(!preg_match('/^\.[a-z0-9]{1,8}$/', $postdata['file_extension'])) {
			$postdata['file_extension'] = "";
		}
		$postdata['tmpname'] = $postdata['tmpname'].$postdata['file_extension'];
		// the title the uploader typed wins over the name of the file they picked.
		// it doubles as the stored file name, so it has to be sanitized either way.
		$title = array_key_exists('filename', $_POST) && is_string($_POST['filename'])
		         ? sanitizeUploadTitle($_POST['filename'])
		         : "";
		if($title == "") {
			$title = sanitizeUploadTitle($_FILES['image']['name']);
		}
		$postdata['filename'] = $title == "" ? "upload" : $title;
		if(array_key_exists('nsfw', $_POST) || strpos(strtolower($postdata['filename']), "[nsfw]") !== false)
			$postdata['nsfw'] = $_POST['nsfw'];
		if(array_key_exists('tmbo', $_POST) || strpos(strtolower($postdata['filename']), "[tmbo]") !== false)
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
			$message = "filenames can't end in .php";
			box($message, "nice try.");
			uploadForm();
			return;
		}
	}

	$type = upload_type($postdata);
	if($type == "image-unsupported") {
		killUpload($postdata);
		box("HEIC and AVIF images aren't supported. Save it as a JPEG and try again.
		     (On an iPhone: Settings &rsaquo; Camera &rsaquo; Formats &rsaquo; Most Compatible.)", "sorry.");
		uploadForm();
		return false;
	}

	switch($type) {
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
	      "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! ".me()->username().
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
		// These container formats come first: the generic RIFF and QuickTime
		// patterns below would match them too, and the first match wins.

		// WebP - still or animated image - stored as uploaded
		'webp' => array(
					'pattern'    => '^RIFF.{4}WEBP',
					'group'      => 'image',
					'extensions' => array('.webp', ),
				  ),

		// HEIF/HEIC and AVIF - still image - ISO-BMFF, identified by the ftyp brand.
		// Not accepted: decoding them would put libheif's HEVC and AV1 decoders
		// in reach of uploads. Detected only so upload() can say what to do
		// instead of the generic rejection (and its suspicious-upload email).
		'heif' => array(
					'pattern'    => '^.{4}ftyp(heic|heix|hevc|hevx|heim|heis|mif1|msf1|avif|avis)',
					'group'      => 'image-unsupported',
				  ),


		// Audio formats

		// AC-3   - audio      - Dolby AC-3 / Dolby Digital
		'ac3'  => array(
					'pattern'    => '^\x0B\x77',
					'group'      => 'audio-forbidden',
				  ),

		// AAC  - audio       - Advanced Audio Coding (AAC) - ADIF format
		'adif' => array(
					'pattern'    => '^ADIF',
					'group'      => 'audio-forbidden',
				  ),


		// AAC  - audio       - Advanced Audio Coding (AAC) - ADTS format (very similar to MP3)
		'adts' => array(
					'pattern'    => '^\xFF[\xF0-\xF1\xF8-\xF9]',
					'group'      => 'audio-forbidden',
				  ),


		// AU   - audio       - NeXT/Sun AUdio (AU)
		'au'   => array(
					'pattern'    => '^\.snd',
					'group'      => 'audio-forbidden',
				  ),

		// AVR  - audio       - Audio Visual Research
		'avr'  => array(
					'pattern'    => '^2BIT',
					'group'      => 'audio-forbidden',
				  ),

		// BONK - audio       - Bonk v0.9+
		'bonk' => array(
					'pattern'    => '^\x00(BONK|INFO|META| ID3)',
					'group'      => 'audio-forbidden',
				  ),

		// FLAC - audio       - Free Lossless Audio Codec
		'flac' => array(
					'pattern'    => '^fLaC',
					'group'      => 'audio-forbidden',
				  ),

		// LA   - audio       - Lossless Audio (LA)
		'la'   => array(
					'pattern'    => '^LA0[2-4]',
					'group'      => 'audio-forbidden',
				  ),

		// LPAC - audio       - Lossless Predictive Audio Compression (LPAC)
		'lpac' => array(
					'pattern'    => '^LPAC',
					'group'      => 'audio-forbidden',
				  ),

		// MIDI - audio       - MIDI (Musical Instrument Digital Interface)
		'midi' => array(
					'pattern'    => '^MThd',
					'group'      => 'audio-forbidden',
				  ),

		// MAC  - audio       - Monkey's Audio Compressor
		'mac'  => array(
					'pattern'    => '^MAC ',
					'group'      => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (assorted sub-formats)
		'mod'  => array(
					'pattern'    => '^.{1080}(M.K.|[5-9]CHN|[1-3][0-9]CH)',
					'group'      => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (Impulse Tracker)
		'it'   => array(
					'pattern'    => '^IMPM',
					'group'      => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (eXtended Module, various sub-formats)
		'xm'   => array(
					'pattern'    => '^Extended Module',
					'group'      => 'audio-forbidden',
				  ),

		// MOD  - audio       - MODule (ScreamTracker)
		's3m'  => array(
					'pattern'    => '^.{44}SCRM',
					'group'      => 'audio-forbidden',
				  ),

		// MPC  - audio       - Musepack / MPEGplus
		'mpc'  => array(
					'pattern'    => '^(MP\+|[\x00\x01\x10\x11\x40\x41\x50\x51\x80\x81\x90\x91\xC0\xC1\xD0\xD1][\x20-37][\x00\x20\x40\x60\x80\xA0\xC0\xE0])',
					'group'      => 'audio-forbidden',
				  ),

		// MP3  - audio       - MPEG-audio Layer 3 (very similar to AAC-ADTS)
		'mp3'  => array(
					'pattern'    => '^\xFF[\xE2-\xE7\xF2-\xF7\xFA-\xFF][\x00-\xEB]',
					'group'      => 'audio',
					'extensions' => array('.mp3', ),
				  ),

		// OFR  - audio       - OptimFROG
		'ofr'  => array(
					'pattern'    => '^(\*RIFF|OFR)',
					'group'      => 'audio-forbidden',
				  ),

		// RKAU - audio       - RKive AUdio compressor
		'rkau' => array(
					'pattern'    => '^RKA',
					'group'      => 'audio-forbidden',
				  ),

		// SHN  - audio       - Shorten
		'shn'  => array(
					'pattern'    => '^ajkg',
					'group'      => 'audio-forbidden',
				  ),

		// TTA  - audio       - TTA Lossless Audio Compressor (http://tta.corecodec.org)
		'tta'  => array(
					'pattern'    => '^TTA',  // could also be '^TTA(\x01|\x02|\x03|2|1)'
					'group'      => 'audio-forbidden',
				  ),

		// VOC  - audio       - Creative Voice (VOC)
		'voc'  => array(
					'pattern'    => '^Creative Voice File',
					'group'      => 'audio-forbidden',
				  ),

		// VQF  - audio       - transform-domain weighted interleave Vector Quantization Format (VQF)
		'vqf'  => array(
					'pattern'    => '^TWIN',
					'group'      => 'audio-forbidden',
				  ),

		// WV  - audio        - WavPack (v4.0+)
		'wv'   => array(
					'pattern'    => '^wvpk',
					'group'      => 'audio-forbidden',
				  ),


		// Audio-Video formats

		// ASF  - audio/video - Advanced Streaming Format, Windows Media Video, Windows Media Audio
		'asf'  => array(
					'pattern'    => '^\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C',
					'group'      => 'audio-video-forbidden',
				  ),

		// BINK  - audio/video - Bink / Smacker
		'bink' => array(
					'pattern'    => '^(BIK|SMK)',
					'group'      => 'audio-video-forbidden',
				  ),

		// MKAV - audio/video - Mastroka
		'matroska' => array(
					'pattern'    => '^\x1A\x45\xDF\xA3',
					'group'      => 'audio-video-forbidden',
				  ),

		// MPEG - audio/video - MPEG (Moving Pictures Experts Group)
		'mpeg' => array(
					'pattern'    => '^\x00\x00\x01(\xBA|\xB3)',
					'group'      => 'audio-video-forbidden',
				  ),

		// NSV  - audio/video - Nullsoft Streaming Video (NSV)
		'nsv'  => array(
					'pattern'    => '^NSV[sf]',
					'group'      => 'audio-video-forbidden',
				  ),

		// Ogg  - audio/video - Ogg (Ogg-Vorbis, Ogg-FLAC, Speex, Ogg-Theora(*), Ogg-Tarkin(*))
		'ogg'  => array(
					'pattern'    => '^OggS',
					'group'      => 'audio-forbidden',
				  ),

		// QT   - audio/video - Quicktime
		'quicktime' => array(
					'pattern'    => '^.{4}(cmov|free|ftyp|mdat|moov|pnot|skip|wide)',
					'group'      => 'audio-video-forbidden',
				  ),

		// RIFF - audio/video - Resource Interchange File Format (RIFF) / WAV / AVI / CD-audio / SDSS = renamed variant used by SmartSound QuickTracks (www.smartsound.com) / FORM = Audio Interchange File Format (AIFF)
		'riff' => array(
					'pattern'    => '^(RIFF|SDSS|FORM)',
					'group'      => 'audio-video-forbidden',
				  ),

		// Real - audio/video - RealAudio, RealVideo
		'real' => array(
					'pattern'    => '^(\.RMF|.ra)',
					'group'      => 'audio-video-forbidden',
				  ),

		// SWF - audio/video - ShockWave Flash
		'swf' => array(
					'pattern'    => '^(F|C)WS',
					'group'      => 'audio-video-forbidden',
				  ),


		// Still-Image formats

		// BMP  - still image - Bitmap (Windows, OS/2; uncompressed, RLE8, RLE4)
		'bmp'  => array(
					'pattern'    => '^BM',
					'group'      => 'image-forbidden',
					'extensions' => array('.bmp', '.dib', ),
				  ),

		// GIF  - still image - Graphics Interchange Format
		'gif'  => array(
					'pattern'    => '^GIF',
					'group'      => 'image',
					'extensions' => array('.gif', ),
				  ),

		// JPEG - still image - Joint Photographic Experts Group (JPEG)
		'jpg'  => array(
					'pattern'    => '^\xFF\xD8\xFF',
					'group'      => 'image',
					'extensions' => array('.jpeg', '.jpg', '.jpe', '.jfif', '.jif', ),
				  ),

		// PCD  - still image - Kodak Photo CD
		'pcd'  => array(
					'pattern'    => '^.{2048}PCD_IPI\x00',
					'group'      => 'image-forbidden',
				  ),


		// PNG  - still image - Portable Network Graphics (PNG)
		'png'  => array(
					'pattern'    => '^\x89\x50\x4E\x47\x0D\x0A\x1A\x0A',
					'group'      => 'image',
					'extensions' => array('.png', ), 
				  ),


		// TIFF  - still image - Tagged Information File Format (TIFF)
		'tiff' => array(
					'pattern'    => '^(II\x2A\x00|MM\x00\x2A)',
					'group'      => 'image-forbidden',
					'extensions' => array('.tif', '.tiff', ),
				  ),


		// Data formats

		// ISO  - data        - International Standards Organization (ISO) CD-ROM Image
		'iso'  => array(
					'pattern'    => '^.{32769}CD001',
					'group'      => 'misc-forbidden',
				  ),

		// RAR  - data        - RAR compressed data
		'rar'  => array(
					'pattern'    => '^Rar\!',
					'group'      => 'archive-forbidden',
				  ),

		// SZIP - audio       - SZIP compressed data
		'szip' => array(
					'pattern'    => '^SZ\x0A\x04',
					'group'      => 'archive-forbidden',
				  ),

		// TAR  - data        - TAR compressed data
		'tar'  => array(
					'pattern'    => '^.{100}[0-9\x20]{7}\x00[0-9\x20]{7}\x00[0-9\x20]{7}\x00[0-9\x20\x00]{12}[0-9\x20\x00]{12}',
					'group'      => 'archive-forbidden',
				  ),

		// GZIP  - data        - GZIP compressed data
		'gz'  => array(
					'pattern'    => '^\x1F\x8B\x08',
					'group'      => 'archive-forbidden',
				  ),

		// ZIP  - data        - ZIP compressed data
		'zip'  => array(
					'pattern'    => '^PK\x03\x04',
					'group'      => 'archive-forbidden',
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
		$intvalue = 0;
		for ($i = 0; $i < $bytewordlen; $i++) {
				$intvalue = $intvalue | (ord($byteword[$i]) & 0x7F) << (($bytewordlen - 1 - $i) * 7);
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
			// do we enforce extensions for this file format?
			if(array_key_exists('extensions', $info)) {
				$current_extension = strpos($postdata['filename'], ".") !== false
				                     ? substr(strtolower($postdata['filename']), strrpos($postdata['filename'], "."))
				                     : "";
				if($current_extension == "") {
					$postdata['filename'] = $postdata['filename'].".".$format_name;
					$postdata['file_extension'] = ".".$format_name;
				} else if(!in_array($current_extension, $info['extensions'])) {
					if(in_array($current_extension, array(".jpg", ".jpeg", ".jpe", ".jfif", ".jif", ".gif", ".png", ".tiff", ".tif", ".bmp", ".dib", ".webp"))) {
						// replace the extension if the upload already has an image extension
						$postdata['filename'] = substr($postdata['filename'], 0, strrpos($postdata['filename'], ".")).".".$format_name;
					} else {
						// append otherwise
						$postdata['filename'] = $postdata['filename'].".".$format_name;
					}
					$postdata['file_extension'] = ".".$format_name;
				}
			}
			$postdata["format_name"] = $format_name;
			if($info['group'] == "image" && (array_key_exists('avatar', $_POST) ||
			                                 array_key_exists('avatar', $postdata))) {
					$postdata['avatar'] = true;
					$info['group'] = "avatar";
			} else if(array_key_exists('avatar', $_POST)) {
				box("avatars must be image files", "hard to look at anything else.");
				return "";
			}
			$upload_identified_key = $format_name;
			$upload_identified_type = $info['group'];
			return $upload_identified_type;
		}
	}

	$foo = new getid3_id3v2($fp, $inf);
	if(count($inf) > 0 && preg_match('/\.mp3$/i', $filename)) {
		// the title may have come from the form without an extension
		if(!preg_match('/\.mp3$/i', $postdata['filename'])) {
			$postdata['filename'] .= ".mp3";
		}
		$upload_identified_key = "mp3";
		$upload_identified_type = "audio";
		return $upload_identified_type;
	}

	return "";
}

###############################################################################
function upload_image($postdata) {
	if(upload_largerthan($postdata, UPLOAD_MAX_BYTES)) return false;
	if(upload_animated_largerthan($postdata, UPLOAD_MAX_POST_BYTES)) return false;
	$lowercase_filename = strtolower($postdata['filename']);
	if(!strstr($lowercase_filename, "[repost]") &&
	   !strstr($lowercase_filename, "[tiar]") &&
	   !strstr($lowercase_filename, "[irlard]")) {
		if(upload_checkisk($postdata)) return false;
		if(upload_checkrepost($postdata)) return false;			// keep as fallback
	}
	if(upload_checkdims($postdata)) return false;
	if(upload_checksize($postdata)) return false;
	return upload_insert_file($postdata);
}

function upload_avatar($postdata) {
	if(upload_largerthan($postdata, UPLOAD_MAX_BYTES)) return false;
	if(upload_animated_largerthan($postdata, UPLOAD_MAX_POST_BYTES)) return false;
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
function upload_checkdims(&$postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic" || upload_type($postdata) == "audio")
		trigger_error("can't check pixel dimensions of non-image files", E_USER_ERROR);

	$original = "$tmpdir/{$postdata['tmpname']}";
	$filesize = filesize($original);
	list($width, $height, $type, $attr) = getimagesize($original);

	if(array_key_exists("noresize", $postdata)) {
		if(upload_resize_required($filesize)) {
			// the form doesn't offer the original, so someone's poking at it.
			unset($postdata['noresize']);
			upload_resize_discard($postdata);
			upload_resize_form($postdata, $width, $height, $filesize);
			return true;
		}
		// they chose the original: drop any preview, and don't follow the
		// resize tool with the compression prompt, they've seen that choice.
		upload_resize_discard($postdata);
		$postdata['noresample'] = true;
		return false;
	}

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

	// the resize tool's buttons. These are one-shot actions, so they're read
	// from the request rather than kept in $postdata like the other flags.
	if(array_key_exists("resize_use", $_POST)) {
		return upload_resize_use($postdata, $width, $height, $filesize);
	}
	if(array_key_exists("resize_preview", $_POST)) {
		return upload_resize_preview($postdata, $width, $height, $filesize);
	}
	if(array_key_exists("resize_retry", $_POST)) {
		upload_resize_discard($postdata);
		upload_resize_form($postdata, $width, $height, $filesize);
		return true;
	}

	if(max($width, $height) <= UPLOAD_PROMPT_MAX_SIDE &&
	   $width * $height <= UPLOAD_PROMPT_MAX_PIXELS &&
	   $filesize <= UPLOAD_PROMPT_MAX_BYTES) {
		return false;
	}

	// we can't shrink an animated image and keep the animation, so don't
	// offer to (upload_animated_largerthan() has already turned away the ones
	// too big to post as they are).
	if(upload_is_animated($postdata))
		return false;

	upload_resize_form($postdata, $width, $height, $filesize);
	return true;
}

/* The resize tool: the uploader picks a long edge (and a quality, for formats
 * that have one), looks at the result, then uses it, tries other settings, or
 * posts the original. The preview sits in the quarantine next to the original
 * until one of those happens (the nightly quarantine sweep takes abandoned
 * ones) and is shown only to its uploader, through offensive/ui/uploadPreview.php.
 */

// formats whose encoder takes a quality setting. PNG and GIF are lossless here,
// so only the size changes.
function upload_resize_has_quality($postdata) {
	return array_key_exists('format_name', $postdata) &&
	       in_array($postdata['format_name'], array("jpg", "webp"));
}

// the preview's file in the quarantine, or "" if there isn't one.
function upload_resize_preview_file($postdata) {
	global $tmpdir;
	if(!array_key_exists('preview', $postdata)) return "";
	$file = $tmpdir."/".$postdata['preview']['tmpname'];
	return file_exists($file) ? $file : "";
}

function upload_resize_discard(&$postdata) {
	$file = upload_resize_preview_file($postdata);
	if($file != "") unlink($file);
	unset($postdata['preview']);
}

// "12.3 MB", "850 KB"
function upload_format_bytes($bytes) {
	if($bytes >= 1024 * 1024) return number_format($bytes / 1024 / 1024, 1)." MB";
	return number_format(max(1, round($bytes / 1024)))." KB";
}

// the settings the form shows: what they last asked for, else the defaults.
function upload_resize_settings($postdata, $width, $height) {
	$longside = max($width, $height);
	$side = array_key_exists('resize_side', $postdata)
	        ? $postdata['resize_side']
	        : min(UPLOAD_RESIZE_DEFAULT_SIDE, $longside);
	$quality = array_key_exists('resize_quality', $postdata)
	           ? $postdata['resize_quality']
	           : UPLOAD_RESIZE_DEFAULT_QUALITY;
	return array($side, $quality, min(UPLOAD_RESIZE_MIN_SIDE, $longside), $longside);
}

function upload_resize_form(&$postdata, $width, $height, $filesize, $error="") {
	list($side, $quality, $minside, $maxside) = upload_resize_settings($postdata, $width, $height);
	$size = upload_format_bytes($filesize);

	ob_start();
	if($error != "") { ?>
		<p><strong><?= $error ?></strong></p>
	<?php } ?>
<?php if(upload_resize_required($filesize)) { ?>
	<p>This image is too big to post as it is (<?= $width ?> x <?= $height ?>, <?= $size ?>;
	the limit is <?= upload_format_bytes(UPLOAD_MAX_POST_BYTES) ?>).<br />
	Pick a size and quality, and I'll make a smaller copy for you to look at before anything is posted.</p>
<?php } else { ?>
	<p>This image is rather large (<?= $width ?> x <?= $height ?>, <?= $size ?>).<br />
	If you want, I can make a smaller copy for you to look at before anything is posted.</p>
<?php } ?>
	<form id="resizeform" action="<?= $_SERVER['SCRIPT_NAME'] ?>" method="POST">
		<input type="hidden" name="c" value="upload"/>
		<label>long edge
			<input type="number" name="resize_side" value="<?= (int)$side ?>"
			       min="<?= $minside ?>" max="<?= $maxside ?>" step="1" required
			       inputmode="numeric"/> px
		</label>
<?php if(upload_resize_has_quality($postdata)) { ?>
		<label>quality <output for="resizequality" hidden><?= (int)$quality ?></output>
			<span>smaller
				<input type="range" name="resize_quality" id="resizequality" value="<?= (int)$quality ?>"
				       min="<?= UPLOAD_RESIZE_MIN_QUALITY ?>" max="100" step="5"/>
				sharper</span>
		</label>
<?php } else { ?>
		<p><?= strtoupper($postdata['format_name']) ?> is lossless, so only the size changes.</p>
<?php } ?>
		<input type="submit" name="resize_preview" value="Show me the resized image."/>
<?php if(!upload_resize_required($filesize)) { ?>
		<input type="submit" name="noresize" value="Post the original." formnovalidate/>
<?php } ?>
		<input type="submit" name="killit" value="Cancel this upload." formnovalidate/>
	</form>
	<?php
	box(ob_get_clean(), "is it 'post your desktop' day?");
	$_SESSION['postdata'] = serialize($postdata);
}

// reads and checks the settings from the form. Returns an error, or "".
function upload_resize_read_settings(&$postdata, $width, $height) {
	list($side, $quality, $minside, $maxside) = upload_resize_settings($postdata, $width, $height);

	$side = filter_var($_POST['resize_side'] ?? "", FILTER_VALIDATE_INT,
	                   array("options" => array("min_range" => $minside, "max_range" => $maxside)));
	if($side === false) {
		return "The long edge has to be a whole number of pixels from $minside to $maxside.";
	}
	$postdata['resize_side'] = $side;

	if(upload_resize_has_quality($postdata)) {
		$quality = filter_var($_POST['resize_quality'] ?? "", FILTER_VALIDATE_INT,
		                      array("options" => array("min_range" => UPLOAD_RESIZE_MIN_QUALITY, "max_range" => 100)));
		if($quality === false) {
			return "The quality has to be a whole number from ".UPLOAD_RESIZE_MIN_QUALITY." to 100.";
		}
		$postdata['resize_quality'] = $quality;
	}
	return "";
}

function upload_resize_preview(&$postdata, $width, $height, $filesize) {
	global $tmpdir;

	upload_resize_discard($postdata);

	$error = upload_resize_read_settings($postdata, $width, $height);
	if($error != "") {
		upload_resize_form($postdata, $width, $height, $filesize, $error);
		return true;
	}

	$side = $postdata['resize_side'];
	$quality = upload_resize_has_quality($postdata) ? $postdata['resize_quality'] : null;
	$previewname = "preview_".$postdata['tmpname'];

	try {
		$image = new Imagick("$tmpdir/{$postdata['tmpname']}");
		if(image_is_animated($image)) {
			// the prompt is never offered for these, so someone's poking at it.
			$image->destroy();
			upload_resize_form($postdata, $width, $height, $filesize, "Animated images can't be resized.");
			return true;
		}
		$image->resizeImage($side, $side, imagick::FILTER_LANCZOS, 1, true);
		if($quality !== null) {
			$image->setImageCompressionQuality($quality);
		}
		$image->writeImage("$tmpdir/$previewname");
		$image->destroy();
	} catch(ImagickException $e) {
		trigger_error("upload resize preview failed: ".$e->getMessage(), E_USER_WARNING);
		if(file_exists("$tmpdir/$previewname")) unlink("$tmpdir/$previewname");
		upload_resize_form($postdata, $width, $height, $filesize,
		                   "Sorry, I couldn't resize that one. Try other settings, or post the original.");
		return true;
	}
	chmod("$tmpdir/$previewname", 0644);

	list($newwidth, $newheight) = getimagesize("$tmpdir/$previewname");
	$newsize = filesize("$tmpdir/$previewname");
	$postdata['preview'] = array(
		'tmpname' => $previewname,
		'userid'  => me()->id(),
	);

	$change = round(100 * ($newsize - $filesize) / $filesize);
	$change = $change < 0 ? "&minus;".abs($change)."%" : "+$change%";
	$toobig = $newsize > UPLOAD_MAX_POST_BYTES;
	$src = "/offensive/ui/uploadPreview.php?v=".filemtime("$tmpdir/$previewname");

	ob_start(); ?>
	<p><a href="<?= $src ?>" target="_blank"><img id="resizepreview" src="<?= $src ?>"
		width="<?= $newwidth ?>" height="<?= $newheight ?>"
		alt="your image, resized"/></a></p>
	<table id="resizestats">
		<tr><th></th><th>dimensions</th><th>file size</th></tr>
		<tr><th>original</th><td><?= $width ?> x <?= $height ?></td><td><?= upload_format_bytes($filesize) ?></td></tr>
		<tr><th>resized</th><td><?= $newwidth ?> x <?= $newheight ?></td><td><?= upload_format_bytes($newsize) ?> (<?= $change ?>)</td></tr>
	</table>
	<p>long edge <?= $side ?> px<?= $quality !== null ? ", quality $quality" : "" ?>.
	The image links to its full size.</p>
<?php if($toobig) { ?>
	<p><strong>That's still too big to post (the limit is <?= upload_format_bytes(UPLOAD_MAX_POST_BYTES) ?>).
	Try a smaller size or a lower quality.</strong></p>
<?php } ?>
	<form id="resizeform" action="<?= $_SERVER['SCRIPT_NAME'] ?>" method="POST">
		<input type="hidden" name="c" value="upload"/>
<?php if(!$toobig) { ?>
		<input type="submit" name="resize_use" value="Use the resized image."/>
<?php } ?>
		<input type="submit" name="resize_retry" value="Try different settings."/>
<?php if(!upload_resize_required($filesize)) { ?>
		<input type="submit" name="noresize" value="Post the original."/>
<?php } ?>
		<input type="submit" name="killit" value="Cancel this upload."/>
	</form>
	<?php
	box(ob_get_clean(), "how's this?");
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

function upload_resize_use(&$postdata, $width, $height, $filesize) {
	global $tmpdir;

	$preview = upload_resize_preview_file($postdata);
	if($preview == "" || $postdata['preview']['userid'] != me()->id()) {
		unset($postdata['preview']);
		upload_resize_form($postdata, $width, $height, $filesize,
		                   "That preview is gone. Have another go?");
		return true;
	}
	if(filesize($preview) > UPLOAD_MAX_POST_BYTES) {
		upload_resize_discard($postdata);
		upload_resize_form($postdata, $width, $height, $filesize,
		                   "That's still too big to post. Try a smaller size or a lower quality.");
		return true;
	}

	// the resized copy takes the original's place and carries on through the
	// rest of the checks. It's already been through the compression choice.
	rename($preview, "$tmpdir/{$postdata['tmpname']}")
		or trigger_error("could not replace the upload with its preview", E_USER_ERROR);
	unset($postdata['preview']);
	$postdata['noresize'] = true;
	$postdata['noresample'] = true;
	return false;
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
		
		$image = new Imagick("$tmpdir/{$postdata['tmpname']}");
		
		if(image_is_animated($image)) {
			// animated GIF. fuck it.
			$postdata['noresample'] = $postdata['resample'];
			unset($postdata['resample']);
			return false;
		}
		
		$image->setImageCompression(imagick::COMPRESSION_JPEG); 
		$image->setImageCompressionQuality(90); 
		if(!$image->writeImage("$tmpdir/$newtmp")) {  
			trigger_error("ImageMagick/writeImage failed", E_USER_ERROR);
		}
		$image->destroy();
		
		// clean up the (potentially) extra file
		if($newfilename != $postdata['filename'])
			unlink("$tmpdir/{$postdata['tmpname']}");

		$postdata['tmpname'] = $newtmp;
		$postdata['filename'] = $newfilename;
		$postdata['file_extension'] = ".jpg";
		$postdata['format_name'] = "jpg";

		$postdata['noresample'] = $postdata['resample'];
		unset($postdata['resample']);

		return false;
	}

	global $filesize, $expectedFileSize;
	if(!needCompression($tmpdir."/".$postdata['tmpname'])) 
		return false;

	$filesize_k = round($filesize / 1024);

	$message = "This image has a pretty big filesize for its dimensions. Expected file size: {$expectedFileSize}k. Your file: {$filesize_k}k.<br />If you want, I can resample it as an 90% quality JPG image.<br /><br />
		<form action=\"{$_SERVER['SCRIPT_NAME']}\" method=\"POST\">
			<input type=\"hidden\" name=\"c\" value=\"upload\">
			<input type=\"submit\" name=\"killit\" value=\"Cancel this upload.\"/>
			<input type=\"submit\" name=\"resample\" value=\"Compress my file.\"/>
			<input type=\"submit\" name=\"noresample\" value=\"Post it anyway.\"/>
		</form>";
	if(array_key_exists('format_name', $postdata) && $postdata['format_name'] == 'gif') $message .= "<br /><br />(If this image is animated, don't click the \"Compress\" button!)";

	box($message, "whoa, nelly.");
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

function upload_checkisk(&$postdata) {
	global $tmpdir;

	if(upload_type($postdata) == "topic")
		trigger_error("isk only works on files", E_USER_ERROR);
	
	if(array_key_exists("repost", $postdata))
		return false;

	if(!isk_enabled()) return false;
	
	// we still need a hash in case we want to revert to the old system
	$postdata['hash'] = getHash($tmpdir."/".$postdata['tmpname']);
	if(strlen($postdata['hash']) < 15) trigger_error("bad hash?", E_USER_ERROR);
	
	$isk = new Isk();
	
	$blob = $isk->img_blob($_SERVER['DOCUMENT_ROOT'] . '/offensive/'.$tmpdir."/".$postdata['tmpname']);
	if($isk->error()) {
		return false;
	}
	
	$iskIds = $isk->find($blob);
	if($isk->error()) {
		return false;
	}
	
	if(count($iskIds) < 1) return false;
	
	$string = implode(',', $iskIds);
	$sql = "SELECT *
			FROM offensive_uploads
			WHERE id IN (" . implode(',', $iskIds) .  ")
			ORDER by timestamp DESC";
			
	$result = tmbo_query( $sql );
	
	if( mysqli_num_rows( $result ) == 0 ) return false;
	
	?>
<div class="contentbox">
	<div class="heading">poster child:</div>
	<div class="bluebox">
	<div style="text-align:center">
	Your file looks similar to:
	<br /><center>
	<table><?php 	
	while($row = mysqli_fetch_assoc($result)) {
		$upload = new Upload($row);
		$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
		
		?><tr class="<?= $css ?>"><?php
			if(upload_type($postdata) == "image" || upload_type($postdata) == "avatar") { ?>
				<td height="100px" width="100px">

					<a href="<?= Link::upload($upload) ?>"
						<?php if($upload->filtered()) { ?>
							onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
						<?php } ?>
						><img name="th<?= $upload->id()?>"
							src="<?= $upload->filtered()
							? "/offensive/graphics/th-filtered.gif" 
							: $upload->thumbURL() ?>"
					/></a>

				</td>
			<?php } ?>
		<td><a href="<?= Link::thread($upload); ?>" target="_blank"><?= htmlEscape($upload->filename()) ?></a></td>
		</tr><?php
	}
	?></table></center><br />
	<form action="<?= $_SERVER['SCRIPT_NAME'] ?>" method="POST">
		<input type="hidden" name="c" value="upload">
		<input type="submit" name="killit" value="Cancel this upload."/>
		<input type="submit" name="repost" value="Post it anyway."/>
	</form>
	</div>
	</div>
</div>

	<?php
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
	if( mysqli_num_rows( $result ) == 0 ) return false;

	$filePath=dirname(realpath($_SERVER['SCRIPT_FILENAME']));

	?>
<div class="contentbox">
	<div class="heading">poster child:</div>
	<div class="bluebox">
	<div style="text-align:center">
	Your file appears to be a repost of:
	<br /><center>
	<table><?php
	while($row = mysqli_fetch_assoc($result)) {
		$upload = new Upload($row);
		$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";

		?><tr class="<?= $css ?>"><?php
			if(upload_type($postdata) == "image" || upload_type($postdata) == "avatar") { ?>
				<td height="100px" width="100px">

					<a href="<?= Link::upload($upload) ?>"
						<?php if($upload->filtered()) { ?>
							onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
						<?php } ?>
						><img name="th<?= $upload->id()?>"
							src="<?= $upload->filtered()
							? "/offensive/graphics/th-filtered.gif" 
							: $upload->thumbURL() ?>"
					/></a>

				</td>
			<?php } ?>
		<td><a href="<?= Link::thread($upload); ?>" target="_blank"><?= htmlEscape($upload->filename()) ?></a></td>
		</tr><?php
	}
	?></table></center><br />
	<form action="<?= $_SERVER['SCRIPT_NAME'] ?>" method="POST">
		<input type="hidden" name="c" value="upload">
		<input type="submit" name="killit" value="Cancel this upload."/>
		<input type="submit" name="repost" value="Post it anyway."/>
	</form>
	</div>
	</div>
</div>

	<?php
	$_SESSION['postdata'] = serialize($postdata);
	return true;
}

// a still image this big can only be posted through the resize tool.
function upload_resize_required($filesize) {
	return $filesize > UPLOAD_MAX_POST_BYTES;
}

// ping reads the frame list without decoding pixels.
function upload_is_animated($postdata) {
	global $tmpdir;
	$image = new Imagick();
	$image->pingImage("$tmpdir/{$postdata['tmpname']}");
	$animated = image_is_animated($image);
	$image->destroy();
	return $animated;
}

// animated images can't be resized, so the posting limit is their upload limit.
function upload_animated_largerthan($postdata, $bytes) {
	global $tmpdir;

	if(filesize($tmpdir."/".$postdata['tmpname']) <= $bytes) return false;
	if(!upload_is_animated($postdata)) return false;

	killUpload($postdata);
	box("animated images can be ".upload_format_bytes($bytes)." at most, try something smaller", "no fishing stories.");
	uploadForm();
	return true;
}

function upload_largerthan($postdata, $bytes) {
	global $tmpdir;

	if(upload_type($postdata) == "topic")
		trigger_error("can't compute size of non-files", E_USER_ERROR);

	$filesize = filesize($tmpdir."/".$postdata['tmpname']);

	if($filesize <= $bytes) return false;

	killUpload($postdata);
	box("that file is too big to upload (the limit is ".upload_format_bytes($bytes)."), try something else", "no fishing stories.");
	uploadForm();
	return true;
}

###############################################################################
function upload_insert_file($postdata) {
	global $link;
	global $tmpdir;

	// it's possible (it's happened in the past!) for the time in sql to differ from PHP
	// use the SQL server's time as the trusted source
	$sql = "SELECT NOW()";
	$res = tmbo_query($sql);
	list($now) = mysqli_fetch_array($res);

	// basePath()
	$timestamp = strtotime($now);
	$year = date( "Y", $timestamp );
	$month = date( "m", $timestamp );
	$day = date( "d", $timestamp );
	$path = get_include_path()."/offensive/uploads/$year/$month/$day/".upload_type($postdata);

	$nsfw = array_key_exists('nsfw', $postdata) ? 1 : 0;
	$tmbo = array_key_exists('tmbo', $postdata) ? 1 : 0;

	if(strlen($postdata['hash']) < 15)
		$postdata['hash'] = getHash($tmpdir."/".$postdata['tmpname']);

	// get a fileid for this upload and prepare the landing site
	$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw,tmbo,hash,type,status )
			   VALUES ( " . $_SESSION['userid'] . ", '" .
			                sqlEscape( $postdata['filename'] ) . "', '" .
			                sqlEscape( $_SERVER['REMOTE_ADDR'] ) . "',
			                $nsfw, $tmbo, '".$postdata['hash']."', '" .
			                upload_type( $postdata ) . "', 'pending')";
	tmbo_query($sql);
	$fileid = mysqli_insert_id($link);

	ensureDirExists("$path");

	###########################################################################
	// image types get thumbnails
	if(upload_type($postdata) == "image" || upload_type($postdata) == "avatar") {
		ensureDirExists("$path/thumbs");

		// generate the thumbnail
		// XXX: this is what's failing
	
		// okay to always tack on the extra file extension because we'll be renaming shortly
		$thfile  = $tmpdir . '/th' . $postdata['tmpname'] . $postdata['file_extension'];

		$image = new Imagick("$tmpdir/{$postdata['tmpname']}");

		if(image_is_animated($image)) {
			$image->flattenImages();
		}

		// phone photos are stored sideways with an EXIF Orientation tag. The
		// upload itself is left as it is (browsers honour the tag), but the
		// thumbnail loses the tag, so turn the thumbnail's pixels instead.
		$image->autoOrient();
		$image->setGravity(imagick::GRAVITY_CENTER);
		
		if(!$image->cropThumbnailImage(133,100)) {
			trigger_error("ImageMagick/cropThumbnailImage failed", E_USER_ERROR);
		}
		$image->setImagePage(0, 0, 0, 0);
		if(!$image->writeImage($thfile)) {
			trigger_error("ImageMagick/writeImage failed", E_USER_ERROR);
		}
		$image->destroy();

		// move into place
		rename($thfile,
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

				// cover art is whatever bytes the MP3 carried: keep it only if it's a
				// format we'd accept as an upload (the ImageMagick policy refuses the
				// rest anyway), and drop it rather than fail the upload if it won't
				// decode.
				$info = @getimagesize($thumbname);
				$allowed = array(IMAGETYPE_JPEG, IMAGETYPE_PNG, IMAGETYPE_GIF, IMAGETYPE_WEBP);
				if(!$info || !in_array($info[2], $allowed)) {
					unlink($thumbname);
				} else if($info[0] > 100 || $info[1] > 100) {
					try {
						$image = new Imagick($thumbname);

						if(image_is_animated($image)) {
							$image->flattenImages();
						}

						$image->setGravity(imagick::GRAVITY_CENTER);
						$image->cropThumbnailImage(100,100);
						$image->setImagePage(0, 0, 0, 0);
						$image->writeImage($thumbname);
						$image->destroy();
					} catch(ImagickException $e) {
						trigger_error("cover art thumbnail failed: ".$e->getMessage(), E_USER_WARNING);
						unlink($thumbname);
					}
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

	$upload = new Upload($fileid);

	if($upload->type() == "avatar") {
		$upload->profilePhoto();
	}

	publishChange($upload->type(), $upload->channels(), $upload->api_data(true));

	if($upload->type() == "image" && isk_enabled()) {
		$isk = new Isk();
		if($isk->add($upload->file(), $upload->id())) {
			$isk->save();
		}
	}

	unset($_SESSION['postdata']);

	$message = "<p>
		Thanks. Your upload can be viewed <a href=\"".Link::upload($upload)."\">here</a>.
	</p>
	<p>
			You may <a href=\"".Link::thread($upload)."\">comment on this file here</a>.
	</p>
	<p>
		<a href=\"".Link::listFor($upload)."\">Back to the list</a>
	</p>";
	box($message, "yummy.");
	if(upload_type($postdata) != "avatar")
		uploadForm(true);
}

function upload_insert_topic() {
	// TODO
}

###############################################################################
function image_is_animated($image) {
	/*
	 * other than actually *using* foreach, PHP doesn't have a way to expose how
	 * many elements a Traversable object contains.
	 * Animated GIFs are Traversable and have more than one element.
	 */
	if(!($image instanceof Traversable)) {return false;}
	$frames = 0;
	foreach($image as $frame) { $frames++; }
	return $frames > 1;
}
?>
