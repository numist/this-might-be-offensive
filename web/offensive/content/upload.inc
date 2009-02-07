<?

	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/tabs.inc" );
	require_once( "offensive/assets/functions.inc" );
// XXX: the reliance on getid3 needs to be removed, the library has a lot of warnings
	require_once( "offensive/getid3/getid3.php");
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	
	global $tmpdir, $filetypes;
	$tmpdir = "quarantine";
	$filetypes = array("jpg", "jpeg", "gif", "png");

/*
 * the upload path follows this state machine:
 * 
   digraph uploads {
   	null -> "repost?" -> killit -> "dead.";
   		"repost?" -> repost;
   			repost -> "rotate?";
   			repost -> "resize?";
   			repost -> "resample?";
   			repost -> "yummy.";
   	null -> "rotate?" -> killit;
   	 	"rotate?" -> rotate -> norotate;
   		"rotate?" -> norotate;
   			norotate -> "resize?";
   			norotate -> "resample?";
   			norotate -> "yummy.";
   	null -> "resize?" -> killit;
   		"resize?" -> resize -> noresize;
   		"resize?" -> noresize;
   			noresize -> "resample?";
   			noresize -> "yummy.";
   	null -> "resample?" -> killit;
   		"resample?" -> resample -> noresample;
   		"resample?" -> noresample -> "yummy.";
   	null -> "yummy.";
   }
 * 
 */

	function start() {
		global $readonly;
		
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);

		if($readonly) {
			header("Location: ./");
		}
		
	}

	function requestDetail() {
		ob_start();
		var_dump( $_SERVER );
		var_dump( $_REQUEST );
		if(array_key_exists("postdetail", $_SESSION))
			var_dump( unserialize($_SESSION['postdetail']) );	
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}
	
	function typeFor( $postdata ) {
		switch( $postdata['format'] ) {
			case "quicktime":
				return "video";
			
			case "mp3":
				return "audio";
			
			default:
				return array_key_exists('avatar', $postdata) && $postdata['avatar'] ? 'avatar' : 'image';
		}
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
		if($_SESSION['userid'] == 143) {
			return 9999;
		}

		$sql = "SELECT count( vote ) AS thecount, vote
					FROM offensive_comments, offensive_uploads
					WHERE vote
						AND offensive_comments.fileid = offensive_uploads.id
						AND offensive_uploads.userid = $uid
						AND offensive_uploads.timestamp > DATE_SUB( NOW(), INTERVAL 6 MONTH )
						AND type='image'
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

	function check_format($extension) {
		global $filetypes;
		
		$reg = "/(".implode("|", $filetypes).")/i";
		
		return (bool)preg_match($reg, $extension);
	}

	function body() {
		global $hash, $tmpdir, $me, $filetypes;
		// is there an incoming file?
		if( uploading_file() ) {
			$postdata = array();
			$postdata['tmpname'] = explode("/", $_FILES['image']['tmp_name']);
			$postdata['tmpname'] = array_pop($postdata['tmpname']);
			// having the file extension helps ImageMagick do its thing.
			$postdata['file_extension'] = getFileExtension($_FILES['image']['name']);
			$postdata['tmpname'] = $postdata['tmpname'].$postdata['file_extension'];
			$postdata['filename'] = $_FILES['image']['name'];

			ensureDirExists($tmpdir);
			move_uploaded_file($_FILES['image']['tmp_name'], $tmpdir."/".$postdata['tmpname']) or trigger_error("", E_USER_ERROR);
			chmod($tmpdir."/".$postdata['tmpname'], 0644);
			
			// determine filetype
			// XXX: for now, shut up the error reporting -- this library needs replacement.
			$errno = error_reporting(0);
			$getID3 = new getID3;
			$fileinfo = $getID3->analyze( $tmpdir."/".$postdata['tmpname'] );
			error_reporting($errno);

			if(!check_format($fileinfo["fileformat"])) {
				box("acceptable file types: ".implode(", ", $filetypes).".", "sorry.");

				mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'] .
				      $postdata['tmpname'], requestDetail(),
				      "From: offensive@thismight.be (this might be offensive)\r\nPriority: urgent" );

				uploadForm();
				return;
			}
			$postdata['format'] = $fileinfo['fileformat'];

			if(strlen($postdata['file_extension']) == 0) {
				$postdata['file_extension'] = ".".$postdata["format"];
				$postdata['filename'] .= $postdata['file_extension'];
				rename($tmpdir."/".$postdata['tmpname'], 
				       $tmpdir."/".$postdata['tmpname'].$postdata['file_extension']) or trigger_error("", E_USER_ERROR);
				$postdata['tmpname'] .= $postdata['file_extension'];
			}

			if(!check_format($postdata['file_extension'])) {
				box("accepted file types: ".implode(", ", $filetypes).".", "sorry.");
				killUpload($postdata);
				unset($postdata);
				return;
			}
			
			if(array_key_exists('nsfw', $_POST))
				$postdata['nsfw'] = $_POST['nsfw'];
			if(array_key_exists('tmbo', $_POST))
				$postdata['tmbo'] = $_POST['tmbo'];
			$postdata['avatar'] = array_key_exists('avatar', $_POST);
			$postdata['redirect'] = array_key_exists('redirect', $_POST) && strlen($_POST['redirect']) > 0 ? 
			                        $_POST['redirect'] : null;
			$postdata['hash'] = getHash($tmpdir."/".$postdata['tmpname']);
		}
		else if(array_key_exists('postdata', $_SESSION) && $postdata = unserialize($_SESSION['postdata']))
		{
			
			// cancel the upload
			if(array_key_exists('killit', $_POST)) {
				box("another one bites the dust", "all right!");
				killUpload($postdata);
				return;
			}
			
			// make sure the postdata gets populated correctly
			{
				$flags = array("rotate", "norotate",
				               "resize", "noresize",
				               "repost",
				               "resample", "noresample");
				foreach($flags as $flag) {
					if(array_key_exists($flag, $_POST))
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
				return;
			}
		}

		if(!isset($postdata) || !$postdata) {
			uploadForm();
			return;
		}
	
	
		/**********************************************************************
		* these blocks ensure that the file being uploaded is not a repost,   *
		* not incredibly large (filesize), not unreasonably big (pixels), and *
		* not sideways.                                                       *
		**********************************************************************/
		
		/**********************************************************************
		 * repost checking
		 * note that this does not check avatars for duplicates.  yearbooks can be reposts, it's fine.
		 */
		if(!array_key_exists("repost", $postdata) && checkRepost($postdata))
			return;
		
		if(typeFor($postdata) == "image" || typeFor($postdata) == "avatar") {
			/**********************************************************************
			 * rotation checking
			 * make sure that if there is rotation data in the EXIF tag that we break if it could be rotated.
			 */
			if(!array_key_exists("rotate", $postdata) && !array_key_exists("norotate", $postdata)
			   && checkRotation($postdata))
				return;
			else if(array_key_exists("rotate", $postdata) && !array_key_exists("norotate", $postdata)
					&& function_exists("exif_read_data")) {
        	
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
			
			if(array_key_exists("norotate", $postdata)) {
				if(file_exists("$tmpdir/th{$postdata['tmpname']}"))
					unlink("$tmpdir/th{$postdata['tmpname']}");
				if(file_exists("$tmpdir/rh{$postdata['tmpname']}"))
					unlink("$tmpdir/rh{$postdata['tmpname']}");
			}
			
			/**********************************************************************
			 * image dimension checking
			 */
			if(!array_key_exists("resize", $postdata) && !array_key_exists("noresize", $postdata) && 
			   checkDim($postdata)) {
				return;
			} else if (array_key_exists("resize", $postdata)) {
				$cmd = "convert -resize 1024x1024 $tmpdir/".$postdata['tmpname']." $tmpdir/".$postdata['tmpname'];
				$err = shell_exec($cmd);
				if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);
				
				$postdata['noresize'] = $postdata['resize'];
				unset($postdata['resize']);
			}
        	
			/**********************************************************************
			 * image size checking
			 */
 			if(!array_key_exists("resample", $postdata) && !array_key_exists("noresample", $postdata) &&
			   checkSize($postdata)) {
 				return;
 			} else if (array_key_exists("resample", $postdata)) {
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
				
				$postdata['noresample'] = $postdata['resample'];
				unset($postdata['resample']);
			}
		}

		/**********************************************************************
		* all tests passed, everything from this point on is relevant to the  *
		* process of actually uploading the file and generating the necessary *
		* metadata in the database and on the filesystem.                     *
		**********************************************************************/
		
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
		$path = get_include_path()."/offensive/uploads/$year/$month/$day/".typeFor($postdata)."/";

		$nsfw = array_key_exists('nsfw', $postdata) ? 1 : 0;
		$tmbo = array_key_exists('tmbo', $postdata) ? 1 : 0;

		if(strlen($postdata['hash']) < 15) trigger_error("no hash!", E_USER_ERROR);

		$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw,tmbo,hash,type,status )
				   VALUES ( " . $_SESSION['userid'] . ", '" . 
				                sqlEscape( $postdata['filename'] ) . "', '" . 
				                $_SERVER['REMOTE_ADDR'] . "', 
				                $nsfw, $tmbo, '".$postdata['hash']."', '" . 
				                typeFor( $postdata ) . "', 'pending')";
		tmbo_query($sql);
		$fileid = mysql_insert_id();
		
		$sql = "INSERT INTO offensive_subscriptions (userid, fileid )
		        VALUES ( " . $_SESSION['userid'] . ", $fileid ) ";
		tmbo_query( $sql );
		

		ensureDirExists("$path");
		
		// image types get thumbnails
		if(typeFor($postdata) == "image" || typeFor($postdata) == "avatar") {
			ensureDirExists("$path/thumbs");
	
			// generate the thumbnail
			$cmd = "convert -resize 100x100 $tmpdir/".$postdata['tmpname']." $tmpdir/th".$postdata['tmpname'];
			$err = shell_exec($cmd);
			if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);

			// move into place
			rename($tmpdir."/th".$postdata['tmpname'], 
			     "$path/thumbs/th$fileid".$postdata['file_extension']) 
				or trigger_error("could not copy thumbnail to $path/thumbs!", E_USER_ERROR);
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
		
		// XXX: index here
		//indexUpload($fileid, $postdata['filename'], $me->username());

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
		if(typeFor($postdata) != "avatar")
			uploadForm(true);
			
		unset($_SESSION['postdata']);
	}

	function uploadForm($fileuploaded=false) {
		$uploadsRemaining = numUploadsRemaining( $_SESSION['userid'] );

		?>
			<div class="heading"><? echo $fileuploaded && $uploadsRemaining !== 0 ? "thank you, sir, may i have another?" : "gimme."?></div>
		
		<div class="bluebox">
		
			<div style="text-align:center">
						
				<?php 
				
					if( $uploadsRemaining !== 0 || $_SESSION['status'] == "admin" ) { 
						$remaining = $_SESSION['status'] == 'admin' ? '∞' : $uploadsRemaining;
					
				?>
		
						<p>You have <? echo $remaining ?> upload<? echo $remaining === 1 ? "" : "s"?> left.</p>
		
						<p>If you haven't already, please take a look at <a href="./?c=faq">the rules</a> before uploading.</p>
		

		
					<form method="post"
							action="<?php echo $_SERVER['PHP_SELF']?>"
							enctype="multipart/form-data"
							onsubmit="return validate()"
					>
		
							<table border="0" cellpadding="4" cellspacing="0" style="text-align:left;margin-left:auto;margin-right:auto">
								<tr>
									<td style="text-align:right"><label for="image">file:</label></td>
									<td><input type="file" name="image" id="image" onchange="setFileName( this )"/></td>
								</tr>	
								<tr>
									<td></td>
									<td><input type="checkbox" id="nsfw" name="nsfw" value="1"/><label for="nsfw">[ nsfw ]</label></td>
								</tr>
								<tr>
									<td></td>
									<td><input type="checkbox" id="tmbo" name="tmbo" value="1"/><label for="tmbo">[ this might be offensive ]</label></td>
								</tr>			
								
								<tr>
									<td></td>
									<td>
										<input type="hidden" name="filename" value=""/>
										<input type="hidden" name="c" value="upload"/>
										<input type="submit" value="upload"/>
									</td>
								</tr>	
							</table>

					</form>

				<?php } else { ?>
				
						<p>Save some for later, man.</p>
						<p><a href="./">index</a></p>
				
				<?php } ?>
			</div>		
		
		</div>
		
		<?

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

	function checkRotation($postdata) {
		global $tmpdir;
	
		// rotation only matters with images and avatars.
		if((typeFor($postdata) != "image" && typeFor($postdata) != "avatar")
		   || !function_exists("exif_read_data"))
			return false;
		
		$filename = $tmpdir."/".$postdata['tmpname'];
		$exif = @exif_read_data($tmpdir."/".$postdata['tmpname']);
		
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

	function checkRepost($postdata) {
		global $hash, $tmpdir, $me;
		
		// avatars are exempt
		if(typeFor($postdata) == "avatar") return false;
		
		// not to be confused with the hash() function in activationFunctions
		$hash = getHash($tmpdir."/".$postdata['tmpname']);
	
	    
		if( strlen( $hash ) < 15 ) {
			trigger_error("hash was < 15 chars!  how?!\n".var_dump($postdata), E_USER_ERROR);
		}
	
		$sql = "SELECT *
				FROM offensive_uploads 
				WHERE hash = '$hash' AND type != 'avatar'
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
			
			?><tr class="<?= $css ?>">
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
	
	function checkDim($postdata) {
		global $tmpdir;
	
		// size only matters with images and avatars.
		if(typeFor($postdata) != "image" && typeFor($postdata) != "avatar")
			return false;
		
		$filesize = filesize($tmpdir."/".$postdata['tmpname']);
		list($width, $height, $type, $attr) = getimagesize($tmpdir."/".$postdata['tmpname']);
		
		// sanity check
		if(!is_numeric( $width ) || !is_numeric($height) || $width * $height == 0 ) {
			box("something's wrong with this image...  it doesnt seem to be valid.", "oops.");
			killUpload($postdata);
			return true;
		}
		
		// outright reject anything of the spaceball.gif variety
		if($width < 5 && $height < 5) {
			$message = "Why would you try to upload a $width x $height pixel image?";
			box($message, "spaceballs alive!");
			killUpload($postdata);
			return true;
		}
		
		// outrageous!
		if($height > 1024 && $width > 1024) {
			$message = "This image is rather large ($width x $height).<br />If you want, I can try resizing it for you.<br/><br/>
			<form action=\"{$_SERVER['PHP_SELF']}\" method=\"POST\">
				<input type=\"hidden\" name=\"c\" value=\"upload\">
				<input type=\"submit\" name=\"killit\" value=\"Cancel this upload.\"/>
				<input type=\"submit\" name=\"resize\" value=\"Resize it for me.\"/>
				<input type=\"submit\" name=\"noresize\" value=\"Post it anyway.\"/>
			</form>";
			box($message, "good pic, but is there any reason why I need to view it on the Jumbotron?");
			$_SESSION['postdata'] = serialize($postdata);
			return true;
		}
	}
	
	function checkSize($postdata) {
		global $tmpdir;
		
		if(typeFor($postdata) != 'image' && typeFor($postdata) != 'avatar')
			return false;
		
		// this stuff is for extra large (uncompressed) images.  not sure if we care as much.
		$filesize = filesize($tmpdir."/".$postdata['tmpname']);
		
		list($width, $height, $type, $attr) = getimagesize($tmpdir."/".$postdata['tmpname']);
		
		$bytesPerPixel = 0.0;
		$expectedFileSize = round(($width * $height * 0.2)/1024);
		$bytesPerPixel = ( 1.0 * $filesize / (1.0 * $width * $height ) );
		$filesize = round($filesize/1024);
		
		if($bytesPerPixel <= 0.45) return false;
		
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
	
	function killUpload($postdata) {
		global $tmpdir;
		unset($_SESSION['postdata']);
		if(file_exists($tmpdir."/".$postdata['tmpname']))
			unlink($tmpdir."/".$postdata['tmpname']);
		uploadForm();
	}

?>