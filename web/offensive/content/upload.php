<?

	require_once( 'activationFunctions.php' );
	require_once( "getid3/getid3.php" );
	require_once( "tabs.php" );
	
	function start() {
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./login.php" );
			exit;
		}

		if( strlen( $_REQUEST['killit'] ) > 0 ) {
			killRepost();
		}
		else if( strlen( $_REQUEST['postit'] ) > 0 ) {
			postAnyway();
		}
	}

	$fileuploaded = false;
	$fileid = null;
	$filePath=dirname(realpath($_SERVER['SCRIPT_FILENAME']));
	$imagePath = $filePath . "/images/picpile/";

	if( $_FILES['image']['name'] != "" && $_FILES['image']['size'] > 0 && ! exceededUploadLimit() ) {

		// hack solution to a problem where files with .php in their names (e.g. xxx.php.jpg)
		// would get executed when the image was requested. 
		$f_name = $_FILES['image']['name'];
		if( strstr( $f_name, ".php" ) !== false ) {
			?>Accepted file types: jpg, gif, png, mp3, ogg, mov, wmv, mp4, mpg.<?php
			mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'], requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			exit;			
		}

		preg_match( "/\.[^\.]+$/", $_FILES['image']['name'], $matches );

		$getID3 = new getID3;
		$fileinfo = $getID3->analyze( $_FILES['image']['tmp_name'] );
	
		if( ! preg_match( "/(jpg|jpeg|gif|png|mp3|ogg|mov|wmv|mpg|quicktime|mp4)/i", $fileinfo["fileformat"] ) ) {
			$quarantine = $filePath . "/quarantine/";		
			$filename = $_FILES['image']['name'];
			$filename = uniqueFilename( $quarantine, stripslashes( $filename ) );
			$destination = $quarantine . $filename;
			move_uploaded_file( $_FILES['image']['tmp_name'], $destination );
			echo "Unaccepted file type. Accepted file types are: jpg, gif, png, mp3, ogg, wmv, mp4, mpg.";
			mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'], requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			exit;
		}

		$file_extension = "";

		if( is_array( $matches ) ) {
			$file_extension = $matches[0];
			if( ! preg_match( "/(jpg|jpeg|gif|png|mp3|ogg|mov|wmv|mpg|mp4)/i", $file_extension ) ) {
				?>Accepted file types: jpg, gif, png, mp3, ogg, mov, wmv, mp4, mpg.<?php
				exit;
			}
		}

		getid3_lib::CopyTagsToComments($fileinfo);

		$imagePath = pathFor( $fileinfo["fileformat"] );

		$link = openDbConnection();

		$filename = strlen( $_REQUEST['filename'] ) > 0 ? $_REQUEST['filename'] : $_FILES['image']['name'];

		$filename = uniqueFilename( $imagePath, stripslashes( filenameFor( $fileinfo, $filename ) ) );

		$nsfw = isset( $_REQUEST['nsfw'] ) ? 1 : 0;
		$tmbo = isset( $_REQUEST['tmbo'] ) ? 1 : 0;
#		$puntme = isset( $_REQUEST['puntme'] ) ? 1 : 0;
#
#		if( $puntme == 1 ) {
#			$sql = "update users set account_status = 'locked' where userid=" .  $_SESSION['userid'] . " limit 1";
#			mysql_query( $sql );
#		}

		if( ! isAvatarUpload() ) {
			// not to be confused with the hash() function in activationFunctions
			$hash = getHash( $_FILES['image']['tmp_name'] );
	
			if( strlen( $hash ) > 15 ) { // 15 is arbitrary. just want to make sure we have a hash
				$sql = "SELECT id, filename, hash
						FROM offensive_uploads 
						WHERE hash = '$hash'
						ORDER by timestamp DESC
						LIMIT 1";
				$result = mysql_query( $sql );
				if( mysql_num_rows( $result ) > 0 ) {
					$repost_row = mysql_fetch_assoc( $result );
				}
			}
		}
		
		$bytesPerPixel = 0;
		$filesize = $_FILES['image']['size'];
		list($width, $height, $type, $attr) = getimagesize( $_FILES['image']['tmp_name'] );
		
		$expectedFileSize = round(($width * $height * .2)/1024);
		if( is_numeric( $filesize ) && is_numeric( $width ) && ($width * $height > 0 ) ) {
			$bytesPerPixel = ( 1.0 * $filesize / (1.0 * $width * $height ) );
			$filesize = round($filesize/1024);
		}

		$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw,tmbo,hash,type,status )
				VALUES ( " . $_SESSION['userid'] . ", '" . mysql_real_escape_string( $filename ) . "', '" . $_SERVER['REMOTE_ADDR'] . "', $nsfw, $tmbo, '$hash', '" . typeFor( $fileinfo["fileformat"] ) . "', 'pending' ) ";

		
		$result = mysql_query( $sql );

#		echo mysql_error();

		$entry_id = mysql_insert_id();

		$sql = "insert into offensive_subscriptions (userid, fileid )
				values ( " . $_SESSION['userid'] . ", $entry_id ) ";

		mysql_query( $sql );
		
		$destination = $imagePath . $filename;

		move_uploaded_file( $_FILES['image']['tmp_name'], $destination );
		
		chmod($destination, 0644);
		
		$curdate = time();
		$year = date( "Y", $curdate );
		$month = date( "m", $curdate );
		$day = date( "d", $curdate );

		$typeName = typeFor( $fileinfo["fileformat"] );
		
		$destDir = "uploads/$year/$month/$day/$typeName";
				
		ensureDirExists( $destDir );

#		copy( $destination, "$filePath/$destDir/$entry_id$file_extension" );
		copy( $destination, "$filePath/$destDir/$filename" );

		if( $typeName == "image" ) {
			ensureDirExists( "$destDir/thumbs" );
#			shell_exec( "convert -resize 100x100 \"$destDir/$entry_id$file_extension\" \"$destDir/thumbs/th$entry_id$file_extension\"" );
			shell_exec( "convert -resize 100x100 \"$destDir/$filename\" \"$destDir/thumbs/th$entry_id$file_extension\"" );
		}

#		mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - file uploaded. " . $_SESSION['username'], $filename . "\n\nhttp://themaxx.com/offensive/pages/pic.php?id=" . $entry_id, "From: offensive@themaxx.com (this might be offensive)" );
		
		$fileuploaded = true;
		$fileid = $entry_id;

	}
	
	function ensureDirExists( $path ) {
	
		# $basePath = "/hsphere/local/home/thismightbe/thismight.be/offensive";
		$basePath = dirname(realpath($_SERVER['SCRIPT_FILENAME']));	
		$curPath = $basePath;
		foreach( explode( '/', $path ) as $dir ) {
			$curPath .= "/$dir";
			if( ! file_exists( $curPath ) ) {
				mkdir( $curPath );
			}

		}

	}
	

	
	function isAvatarUpload() {
		return isset( $_REQUEST['avatar'] );
	}
	
	function filenameFor( $fileinfo, $default ) {
		
		if( $fileinfo["fileformat"] == "mp3" || $fileinfo["fileformat"] == "ogg" ) {
			if( isset( $fileinfo['comments_html']['artist'] ) && isset( $fileinfo['comments_html']['title'] ) ) {
				return str_replace( "/", "-",  (implode( $fileinfo['comments_html']['artist'], " " ) . " - " . implode( $fileinfo['comments_html']['title'], "" ) ) . "." . $fileinfo["fileformat"] );
			}
		}
		
		if( isAvatarUpload() ) {
			return $_SESSION['userid'] . "." . $fileinfo["fileformat"];
		}

		return $default;
	}

	function typeFor( $format ) {
		if( isAvatarUpload() ) {
			return "avatar";
		}
		switch( $format ) {
			case "quicktime":
				return "video";
			break;
			
			case "mp3":
			case "ogg":
				return "audio";
			break;
			
			default:
				return "image";
		}
		return "image";
	}

	function pathFor( $format ) {
		global $filePath;
		
		if( isAvatarUpload( $_REQUEST['avatar'] ) ) {
			return $filePath . "/images/users/";
		}
		
		$dir = "/images/picpile/";
		
		switch( typeFor( $format ) ) {
			case "audio":
				$dir = "/images/audio/";
			break;
			
			case "video":
				$dir =  "/images/video/";
			break;
			
			default:
				$dir = "/images/picpile/";

		}
		
		return $filePath . $dir;
	}

	function getHash( $filePath ) {
		$file = fopen( $filePath, "r" );
		$filedata = fread ( $file, min( 4096, filesize( $filePath ) ) );
		fclose( $file );
		return md5( $filedata );
	}

	function requiresLogin() {
		return true;
	}

	function exceededUploadLimit() {
		return (numUploadsRemaining( $uid ) == 0);
	}

	function numUploadsRemaining( $uid ) {
		$limit = numAllowedUploads( $_SESSION['userid'] );
		$used = numUploadsToday( $_SESSION['userid'] );
		return ($limit - $used ) > 0 ? ($limit - $used ) : 0;
	}

	function numUploadsToday( $uid ) {

		$link = openDbConnection();
	
		$sql = "SELECT count( id ) as thecount FROM offensive_uploads WHERE userid = " . $_SESSION['userid'] . " AND timestamp > DATE_SUB( NOW(), INTERVAL 1 DAY ) AND type='image'";
		$result = mysql_query( $sql );

		$row = mysql_fetch_assoc( $result );

		return $row[ 'thecount' ];

	}

	function numAllowedUploads( $uid ) {
	
		return 40;
	
		$link = openDbConnection();

		$sql = "SELECT count( vote ) AS thecount, vote
					FROM offensive_comments, offensive_uploads
					WHERE vote
						AND offensive_comments.fileid = offensive_uploads.id
						AND offensive_uploads.userid = $uid
						AND offensive_uploads.timestamp > DATE_SUB( NOW(), INTERVAL 6 MONTH )
						AND type='image'
					GROUP  BY vote";

		$result = mysql_query( $sql );
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
		$x = 1;
		while( file_exists( $imageDir . $filename ) && $x++ < 1000 ) {
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
		$link = openDbConnection();
		$sql = "update offensive_uploads set status='$newstatus' where id=$fileid limit 1";
		mysql_query( $sql );
	}

	function body() {
		global $fileuploaded, $fileid, $repost_row, $bytesPerPixel, $filesize, $expectedFileSize, $width, $height, $hash;

		if( isset( $repost_row ) ) {
			$message = "Your file appears to be a repost of <a target=\"_blank\" href=\"pages/pic.php?id=" . $repost_row['id'] . "\">" . $repost_row['filename'] . "</a>.";
			$hash = $repost_row['hash'];
			repostForm( $message, $hash );
		}
		else if( $bytesPerPixel > .45 ) {
			$message = "Your upload exceeds the recommended file size for an image of its dimensions. Expected file size: ${expectedFileSize}k. Your file: ${filesize}k. (If you'd like to compress it but don't have the tools, you might try running it through <a target=\"_blank\" href=\"http://mmppt.porkchop.net\">porkchop's magic maxx photo prep thing</a>. (thanks porkchop!))<br/>Post it anyway?</br>";
			repostForm( $message, $hash );			
		}
		else if( $width > 1200 || $height > 1200 ) {
			$message = "This image is rather large ($width x $height). (If you'd like to resize it but don't have the tools, you might try running it through <a target=\"_blank\" href=\"http://mmppt.porkchop.net\">porkchop's magic maxx photo prep thing</a>. (thanks porkchop!))<br/>Post it anyway?";
			repostForm( $message, $hash );			
		}
		else {

			if( $fileuploaded ) {
				updateStatus( $fileid, 'normal' );
				thanks();		
			}

			uploadForm();

		}

	} 


function thanks() {
	global $fileid;
	
	if( $_REQUEST['redirect'] ) {
		header( "Location: " . $_REQUEST['redirect'] );
	}
?>
	<div class="heading">yummy.</div>

	<div class="bluebox">
	
		<div style="text-align:center">
		
			
				<p>
					Thanks. Your upload can be viewed <a href="./pages/pic.php?id=<?= $fileid?>">here</a>.
				</p>
				<? if( isset( $fileid ) ) { ?>
					<p>
						You may <a href="./?c=comments&fileid=<? echo $fileid ?>">comment on this file here</a>.
					</p>
				<? } ?>
				
				<p>
					<a href="./">Back to the list</a>
				</p>

		</div>		
	
	</div>

	<div class="blackbar"></div>

<?
}

function brb() {

	?>
	<div class="bluebox">
		no uploads right now. be back in a bit. (hopefully not more than 15 minutes).
	</div>
	<?

}

function uploadForm() {

		?>
			<div class="heading"><? echo $fileuploaded ? "thank you, sir, may i have another?" : "gimme."?></div>
		
		<div class="bluebox">
		
			<div style="text-align:center">
						
				<?php 
				
					$uploadsRemaining = numUploadsRemaining( $_SESSION['userid'] );
					
				
					if( $uploadsRemaining > 0 ) { 
					
				?>
		
						<!-- <p>You have <? echo $uploadsRemaining ?> upload<? echo $uploadsRemaining == 1 ? "" : "s"?> left.</p> -->
		
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
								<!--
								<tr>
									<td></td>
									<td><input type="checkbox" id="puntme" name="puntme" value="1"/><label for="puntme">[ please ban me and lock my account. ]</label></td>
								</tr>								
								-->				
								
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


function repostForm( $message, $hash ) {
	global $fileid;

	$rehash = tmbohash( $fileid, $hash );
?>

	<div class="heading">poster child:</div>
	<div class="bluebox">
		<?= $message ?>
		<form action="<?= $_SERVER['PHP_SELF'] ?>">
			<input type="hidden" name="c" value="upload">
			<input type="hidden" name="repost" value="<?= $rehash ?>"/>
			<input type="submit" name="killit" value="Cancel this upload."/>
			<input type="submit" name="postit" value="Post it anyway."/>
		</form>
	</div>

<?		

}

function killRepost() {
	
	$repostId = id_from_hash( $_REQUEST['repost'] );
	
	if( ! is_numeric( $repostId ) ) {
		return;	
	}
	
	$sql = "SELECT hash
				FROM offensive_uploads
				WHERE id = $repostId
				AND userid = " . $_SESSION['userid'] . "
				LIMIT 1";

	$link = openDbConnection();
	$result = mysql_query( $sql );
	if( mysql_num_rows( $result ) > 0 ) {
		
		$row = mysql_fetch_assoc( $result );
		$dbHash = $row['hash'];

		$rehash = tmbohash( $repostId, $dbHash );

		if( $rehash == $_REQUEST['repost'] ) {

			$sql = "DELETE FROM offensive_uploads
						WHERE id=$repostId
						AND hash = '$dbHash'
						AND userid=" . $_SESSION['userid'] . "
						LIMIT 1";

			mysql_query( $sql );

		}

	}

}


function postAnyway() {
	
	$repostId = id_from_hash( $_REQUEST['repost'] );
	
	if( ! is_numeric( $repostId ) ) {
		return;	
	}
	
	$sql = "SELECT hash
				FROM offensive_uploads
				WHERE id = $repostId
				AND userid = " . $_SESSION['userid'] . "
				LIMIT 1";

	$link = openDbConnection();
	$result = mysql_query( $sql );
	if( mysql_num_rows( $result ) > 0 ) {

		$row = mysql_fetch_assoc( $result );
		$dbHash = $row['hash'];

		$rehash = tmbohash( $repostId, $dbHash );

		if( $rehash == $_REQUEST['repost'] ) {
			updateStatus( $repostId, 'normal' );
		}

	}

}


?>
