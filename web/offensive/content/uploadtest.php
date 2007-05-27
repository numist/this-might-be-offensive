<?

	require_once( 'activationFunctions.php' );

	function start() {
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./login.php" );
			exit;
		}

		if( strlen( $_REQUEST['killit'] ) > 0 ) {
			killRepost();
		}
	}

	$fileuploaded = false;
	$fileid = null;
	
	# realpath suddenly stopped working 2005-05-27
#	$filePath=dirname(realpath($_SERVER['SCRIPT_FILENAME']));
	$filePath = "/home/.chippy/fleece/themaxx.com/offensive";
	$imagePath = $filePath . "/images/picpile/";

	if( $_FILES['image']['name'] != "" && $_FILES['image']['size'] > 0 && ! exceededUploadLimit() ) {
	
		preg_match( "/\.[^\.]+$/", $_FILES['image']['name'], $matches );

		if( ! getimagesize( $_FILES['image']['tmp_name'] ) ) {

			$quarantine = $filePath . "/quarantine/";		
			$filename = $_FILES['image']['name'];
			$filename = uniqueFilename( $quarantine, stripslashes( $filename ) );
			$destination = $quarantine . $filename;
			move_uploaded_file( $_FILES['image']['tmp_name'], $destination );
			echo "Unaccepted file type. Your actions are being monitored.";
			mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - SUSPICIOUS UPLOAD!!! " . $_SESSION['username'], requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			exit;
		}

		$file_extension = "";

		if( is_array( $matches ) ) {
			$file_extension = $matches[0];
			if( ! preg_match( "/(jpg|jpeg|gif|png)/i", $file_extension ) ) {
				?>Accepted file types: jpg, gif, png.<?php
				exit;
			}
		}
	
		$link = openDbConnection();

		$filename = strlen( $_REQUEST['filename'] ) > 0 ? $_REQUEST['filename'] : $_FILES['image']['name'];

		$filename = uniqueFilename( $imagePath, stripslashes( $filename ) );

		$nsfw = isset( $_REQUEST['nsfw'] ) ? 1 : 0;
		$tmbo = isset( $_REQUEST['tmbo'] ) ? 1 : 0;		

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


		$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw,tmbo,hash,type )
				VALUES ( " . $_SESSION['userid'] . ", '" . mysql_real_escape_string( $filename ) . "', '" . $_SERVER['REMOTE_ADDR'] . "', $nsfw, $tmbo, '$hash', 'image' ) ";

		
		$result = mysql_query( $sql );

#		echo mysql_error();

		$entry_id = mysql_insert_id();

		$sql = "insert into offensive_subscriptions (userid, fileid )
				values ( " . $_SESSION['userid'] . ", $entry_id ) ";

		mysql_query( $sql );
		
		$destination = $imagePath . $filename;
		
		move_uploaded_file( $_FILES['image']['tmp_name'], $destination );
		
		chmod($destination, 0644);
		
#		mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - file uploaded. " . $_SESSION['username'], $filename . "\n\nhttp://themaxx.com/offensive/pages/pic.php?id=" . $entry_id, "From: offensive@themaxx.com (this might be offensive)" );
		
		$fileuploaded = true;
		$fileid = $entry_id;

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
	
		$link = openDbConnection();

		$sql = "SELECT count( vote ) AS thecount, vote
					FROM offensive_comments, offensive_uploads
					WHERE vote
						AND offensive_comments.fileid = offensive_uploads.id
						AND offensive_uploads.userid = $uid
						AND offensive_uploads.timestamp > DATE_SUB( NOW(), INTERVAL 3 MONTH )
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

?>


<?

	function body() {
		global $fileuploaded, $fileid, $repost_row;
		
		if( isset( $repost_row ) ) {

			repostForm( $repost_row );

		}
		else {

			if( $fileuploaded ) {
				thanks();		
			}

			uploadForm();

		}

	} 


function thanks() {
	global $fileid;
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
		
						<p>You have <? echo $uploadsRemaining ?> upload<? echo $uploadsRemaining == 1 ? "" : "s"?> left.</p>
		
						<p>If you haven't already, please take a look at <a href="./?c=faq">the rules</a> before uploading.</p>
		

		
					<form method="post"
							action="<?php echo $_SERVER['PHP_SELF']?>"
							enctype="multipart/form-data"
							onsubmit="return validate()"
					>
		
							<table border="0" cellpadding="4" cellspacing="0" style="text-align:left;margin-left:auto;margin-right:auto">
								<tr>
									<td style="text-align:right"><label for="image">image:</label></td>
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
										<input type="hidden" name="c" value="uploadtest"/>							
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


function repostForm( $repost_row ) {
	global $fileid;

	$rehash = hash( $fileid, $repost_row['hash'] );
?>

	<div class="heading">repost?</div>
	<div class="bluebox">
		Your file appears to be a repost of <a target="_blank" href="pages/pic.php?id=<?= $repost_row['id'] ?>"><?= $repost_row['filename'] ?></a>.
		<form action="<?= $_SERVER['PHP_SELF'] ?>">
			<input type="hidden" name="c" value="upload">
			<input type="hidden" name="repost" value="<?= $rehash ?>"/>
			<input type="submit" name="killit" value="kill it"/>
			<input type="submit" value="leave it"/>
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

		$rehash = hash( $repostId, $dbHash );

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

?>



