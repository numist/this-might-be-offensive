<?

	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/tabs.inc" );
	require_once( "offensive/assets/functions.inc" );
// XXX: the reliance on getid3 needs to be removed, the library has a lot of warnings
	require_once( "offensive/getid3/getid3.php");
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/uploads.inc');
	

	$tmpdir = "quarantine";

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
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}
	
	function isAvatarUpload($postdata) {
		if(!array_key_exists('avatar', $postdata)) return false;
		return $postdata['avatar'];
	}
	
	function typeFor( $postdata ) {

		/*
		 * a bug was present in this code that allowed people to upload
		 * arbitrary accepted file types as their avatars.
		 * since we do not actually allow people to use music or video as
		 * their avatars, the check was moved to the default: block of the
		 * switch statement below.
		 */

		switch( $postdata['format'] ) {
			case "quicktime":
				return "video";
			break;
			
			case "mp3":
			case "ogg":
				return "audio";
			break;
			
			default:
				if( isAvatarUpload($postdata) ) {
					return "avatar";
				}
				return "image";
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

	
	// appends or increments a numeric filename suffix to ensure a unique filename
	function uniqueFilename( $imageDir, $original ) {
		$filename = $original;
		while( file_exists( $imageDir ."/". $filename )) {
			$filename = incrementSuffix( $filename );
		}
		return $filename;
	}
	
	function incrementSuffix( $input ) {
		$suffix = ( preg_match( "/([0-9]+)\.(jpg|gif|png)$/", $input, $matches ) == 1) ? $matches[1] : 1;		
		$suffix++;
		$result = preg_replace( "/([0-9]*)\.(jpg|gif|png)$/", "$suffix.\${2}", $input );
		return $result;

	}
	
	function dumpQuery( $sql ) {
		
		$LOGFILE = fopen( "queryLog.txt", "a" );
		if( $LOGFILE ) {
			fwrite( $LOGFILE, $sql . "\n" );
			fclose( $LOGFILE );
		}
		
	}
	
	function updateStatus( $fileid, $newstatus ) {
		$sql = "update offensive_uploads set status='$newstatus' where id=$fileid limit 1";
		tmbo_query( $sql );
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

	function body() {
		global $hash, $tmpdir, $me;
		$postdata = array();
		
		// is there an incoming file?
		if( uploading_file() ) {
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

			if(!preg_match( "/(jpg|jpeg|gif|png)/i", $fileinfo["fileformat"])) {
				box("acceptable file types: jpg, gif, png.", "sorry.");

				mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'] . $postdata['tmpname'], requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );

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

			if( ! preg_match( "/(jpg|jpeg|gif|png)/i", $postdata['file_extension'] ) ) {
				box("accepted file types: jpg, gif, png.", "sorry.");
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
			
		} else if(array_key_exists('postdata', $_SESSION) && $postdata = unserialize($_SESSION['postdata'])) {
			
			if(array_key_exists('killit', $_POST)) {
				box("another one bites the dust", "all right!");
				killUpload($postdata);
				return;
			}
			
			if(array_key_exists('resize', $_POST))
				$postdata['resize'] = $_POST['resize'];
			if(array_key_exists('noresize', $_POST))
				$postdata['noresize'] = $_POST['noresize'];
			if(array_key_exists('repost', $_POST))
				$postdata['repost'] = $_POST['repost'];			
			if(array_key_exists('resample', $_POST))
				$postdata['resample'] = $_POST['resample'];			
			if(array_key_exists('noresample', $_POST))
				$postdata['noresample'] = $_POST['noresample'];			
			
			// if the file doesn't exist, we're not doing any posting, are we...
			if(!file_exists($tmpdir."/".$postdata['tmpname'])) {
				trigger_error("post from SESSION failed: file not found.", E_USER_WARNING);
				$message = "couldn't find the file you wanted to upload, sorry.";
				box($message);
				killUpload($postdata);
				unset($postdata);
				return;
			}
		} else {
			unset($postdata);
		}

		if(!isset($postdata) || !$postdata) {
			uploadForm();
			return;
		}
	
	
		/**********************************************************************
		* these blocks ensire that the file being uploaded is not a repost,   *
		* not incredibly large (filesize), and not unreasonably big (pixels)  *
		**********************************************************************/
		/*
		 * repost checking
		 * note that this does not check avatars for duplicates.  yearbooks can be reposts, it's fine.
		 */
		if(!array_key_exists("repost", $postdata) && checkRepost($postdata))
			return;
		
		/*
		 * image dimension checking
		 */
		if(!array_key_exists("resize", $postdata) && !array_key_exists("noresize", $postdata) && 
		   checkDim($postdata)) {
			return;
		} else if (array_key_exists("resize", $postdata)) {
			$cmd = "convert -resize 1024x1024 $tmpdir/".$postdata['tmpname']." $tmpdir/".$postdata['tmpname'];
			$err = shell_exec($cmd);
			if(strlen($err) > 0) trigger_error("ImageMagick failed: $err", E_USER_ERROR);
		}

		/*
		 * image size checking
		 */
 		if(!array_key_exists("resample", $postdata) && !array_key_exists("noresample", $postdata) &&
		   checkSize($postdata)) {
 			return;
 		} else if (array_key_exists("resample", $postdata)) {
			// convert from file.ext to file.jpg, quality 80
			$newtmp = substr($postdata['tmpname'], 0, 0 - strlen($postdata['file_extension'])).".jpg";
			$newfilename = substr($postdata['filename'], 0, 0 - strlen($postdata['file_extension'])).".jpg";
			
			$cmd = "convert -quality 80 $tmpdir/".$postdata['tmpname']." $tmpdir/$newtmp";
			$err = shell_exec($cmd);
			if(strlen($err) > 0) {
				trigger_error("ImageMagick failed with: $err", E_USER_ERROR);
			}
			$postdata['tmpname'] = $newtmp;
			$postdata['filename'] = $newfilename;
			$postdata['file_extension'] = ".jpg";
			$postdata['format'] = "jpg";
		}

		/**********************************************************************
		* all tests passed, everything from this point on is relevant to the  *
		* process of actually uploading the file and generating the necessary *
		* metadata in the database and on the filesystem.                     *
		**********************************************************************/
		
		/*
		 * get the current time from the database instead of from the server
		 * this solves the time zone problems that occur when we put a file in
		 * the uploads directory for one date, and the database is already in 
		 * the next (or previous) day.  in tmbo's case, the times are 
		 * traditionally 2 hours apart or so, and issues occur when moving
		 * between servers.
		 */ 
		$sql = "SELECT NOW()";
		$res = tmbo_query($sql);
		list($now) = mysql_fetch_array($res);
		$path = basePath($now, typeFor($postdata));

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
		
		indexUpload($fileid, $postdata['filename'], $me->username());

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
		unset($_SESSION['postdata']);
		uploadForm(true);

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

	function checkRepost($postdata) {
		global $hash, $tmpdir, $me;
		
		// avatars are exempt
		if(isAvatarUpload($postdata)) return false;
		
		// not to be confused with the hash() function in activationFunctions
		$hash = getHash($tmpdir."/".$postdata['tmpname']);
	
	    
		if( strlen( $hash ) < 15 ) {
			trigger_error("hash was < 15 chars!  how?!\n".var_dump($postdata), E_USER_ERROR);
		}
	
		$sql = "SELECT id, filename, timestamp, type, nsfw, tmbo, userid
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
			$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
			
			$repdate = strtotime($row['timestamp']);
			?><tr class="<?= $css ?>">
			<td><a href="/offensive/pages/pic.php?id=<?= $row['id'] ?>" target="_blank"><?
			if($me->squelched($row['userid']) || 
			    ($row["nsfw"] == 1 && $me->getPref("hide_nsfw") == 1) || 
			    ($row["tmbo"] == 1 && $me->getPref("hide_tmbo") == 1)) {
				?>[ filtered ]<?
			} else {
				?><img src="<?= getThumbURL($row['id'], $row['filename'], $row['timestamp'], $row['type']) ?>"><?
			}
			?></a></td>
			<td><a href="/offensive/?c=comments&fileid=<?= $row['id'] ?>" target="_blank"><?= $row['filename'] ?></a></td>
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
		
		$message = "This image has a pretty big filesize for its dimensions. Expected file size: ${expectedFileSize}k. Your file: ${filesize}k.<br />If you want, I can resample it as an 80% quality JPG image.<br /><br />
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