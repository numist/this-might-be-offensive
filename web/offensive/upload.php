<?php
	session_start();
	
	require '../admin/mysqlConnectionInfo.php';

	if( ! isset( $_SESSION['userid'] ) ){
		header( "Location: ./login.php" );
		exit;
	}
	
	$fileuploaded = false;
	$fileid = null;
	
	$filePath=dirname(realpath($_SERVER['SCRIPT_FILENAME']));
	$imagePath = $filePath . "/images/picpile/";

	if( $_FILES['image']['name'] != "" && $_FILES['image']['size'] > 0 && ! exceededUploadLimit() ) {
	
		preg_match( "/\.[^\.]+$/", $_FILES['image']['name'], $matches );

		$file_extension = "";

		if( is_array( $matches ) ) {
			$file_extension = $matches[0];
			if( ! preg_match( "/(jpg|gif|png)/i", $file_extension ) ) {
				?>Accepted file types: jpg, gif, png.<?php
				exit;
			}
		}
	
		$link = openDbConnection();

		$filename = strlen( $_REQUEST['filename'] ) > 0 ? $_REQUEST['filename'] : $_FILES['image']['name'];

		$filename = uniqueFilename( $imagePath, stripslashes( $filename ) );

		$nsfw = isset( $_REQUEST['nsfw'] ) ? 1 : 0;

		$sql = "INSERT INTO offensive_uploads ( userid,filename,ip,nsfw )
				VALUES ( " . $_SESSION['userid'] . ", '" . mysql_real_escape_string( $filename ) . "', '" . $_SERVER['REMOTE_ADDR'] . "', " . $nsfw . " ) ";

		
		$result = mysql_query( $sql );

		echo mysql_error();

		$entry_id = mysql_insert_id();
		
		$destination = $imagePath . $filename;
		
		move_uploaded_file( $_FILES['image']['tmp_name'], $destination );
		
		chmod($destination, 0644);
		
		mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - file uploaded. " . $_SESSION['username'], $filename . "\n\nhttp://themaxx.com/offensive/pages/pic.php?id=" . $entry_id, "From: offensive@themaxx.com (this might be offensive)" );
		
		$fileuploaded = true;
		$fileid = $entry_id;
		
#		header("Location: http://".$_SERVER['HTTP_HOST']
#                      .dirname($_SERVER['PHP_SELF'])
#                      ."/thanks.php?fileid=$entry_id");
#                      
#        exit;

	}
	
	function exceededUploadLimit() {
	
		$limit = 8;
	
		$link = openDbConnection();
	
		$sql = "SELECT count( id ) as thecount FROM offensive_uploads WHERE userid = " . $_SESSION['userid'] . " AND timestamp > DATE_SUB( NOW(), INTERVAL 1 DAY )";
		$result = mysql_query( $sql );

		$row = mysql_fetch_assoc( $result );

		return ( $row[ 'thecount' ] > $limit );
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


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 7.0.3">

	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>

	<script type="text/javascript">
	
		// there's an issue in our version of php where
		// filenames containing apostrophes get clipped.
		// to work around this issue, we store the original
		// filename in a hidden field. this function strips
		// the leading path information and sets the value
		// of the hidden field.
		function setFileName( field ) {
			
			var value = "" + field.value;
			
			var result;
			var delimiter = ( "" + value ).indexOf( "\\" ) > 0 ? "\\" : "/";
			
			var filename = value.substring( value.lastIndexOf( delimiter ) + 1, value.length );
			
			var nameField = field.form.elements['filename'];
			nameField.value = filename;
			
		}
	
	</script>
</head>


<body>

 <?php include( $DOCUMENT_ROOT . "/includes/headerbuttons.txt" );?>
 <br/>

<form method="post"
		action="<?php echo $_SERVER['PHP_SELF']?>"
		enctype="multipart/form-data"
		onsubmit="return validate()"
>


<div style="padding:48px;">

<?php if( $fileuploaded ) { ?>

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading">Upload:</div>
			<div class="bluebox">			
			
				<p>
					Thanks. It may be a few minutes before your file is displayed.
				</p>
				<? if( isset( $fileid ) ) { ?>
					<p>
						You may <a href="comments.php?fileid=<? echo $fileid ?>">comment on this file</a> before it appears on the site.
					</p>
				<? } ?>
				
				<p>
					<a href="./">Back to the list</a>
				</p>
			</div>			
		<div class="blackbar"></div>
	</div>
<? } ?>

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading"><? echo $fileuploaded ? "thank you, sir, may i have another?" : "gimme."?></div>
			<div class="bluebox">			
			
			<?php if( ! exceededUploadLimit() ) { ?>
			
				<p>If you haven't already, please take a look at <a href="page.php?c=faq">the rules</a> before uploading.</p>
			
				<p>
					image: <input type="file" name="image" onchange="setFileName( this );"/><br/>
					<input type="hidden" name="filename" value=""/>
					<input type="checkbox" id="nsfw" name="nsfw" value="1"/><label for="nsfw">[ nsfw ]</label>
				</p>

				<p>
					<input type="submit" value="go"/>
				</p>
				
				
			<?php } else { ?>
			
					<p>Save some for later, man.</p>
					<p><a href="./">index</a></p>
			
			<?php } ?>
				
			</div>
		<div class="blackbar"></div>
	</div>

</div>

</form>


</body>
</html>
