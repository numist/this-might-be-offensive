<?php session_start();

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../logn.php" );
		exit;
	}

	$id = $_REQUEST['id'];
	
	$lastpic = $_COOKIE['lastpic'];
	
	if( ! is_numeric( $lastpic ) || $id > $lastpic ) {
		setcookie( 'lastpic', "$id", time()+3600 * 24 * 365, "/offensive/" );
	}

	require_once( '../../admin/mysqlConnectionInfo.php' );
	$link = openDbConnection();
	
	$id = $_REQUEST['id'];
	$uid = $_REQUEST['uid'];

	function writeNav( $id ) {
	
		global $filename, $nsfw, $tmbo, $uploader, $uploaderid;
		
		if( file_exists( "nav/$id.php" ) ) {
			include( "nav/$id.php" );
		}

		if( ! is_numeric( $nextid ) ) {
			
			$link = openDbConnection();

			$sql = "SELECT offensive_uploads.*, users.username, users.userid
					FROM offensive_uploads, users
					WHERE id = $id 
						AND offensive_uploads.userid = users.userid
						AND type='image' AND users.account_status != 'locked'
					LIMIT 1";
										
			$result = mysql_query( $sql );
			$row = mysql_fetch_assoc( $result );
			
			$filename = $row['filename'];

			$nsfw = $row['nsfw'];
			$previd = $row['previd'];		
			$tmbo = $row['tmbo'];
			$uploader = $row['username'];
			$uploaderid = $row['userid'];
			
			$sql = "select min( id ) as nextid from offensive_uploads where id > $id AND type='image' and status='normal'";
			$result = mysql_query( $sql );
			if( mysql_num_rows( $result ) > 0 ) {
				$row = mysql_fetch_assoc( $result );
				$nextid = $row['nextid'];
			}

			$sql = "select max( id ) as previd from offensive_uploads where id < $id AND type='image' and status='normal'";			
			$result = mysql_query( $sql );
			if( mysql_num_rows( $result ) > 0 ) {
				$row = mysql_fetch_assoc( $result );
				$previd = $row['previd'];
			}
			
			if( is_numeric( $nextid ) ) {
				writeNavFile( "nav/$id.php", $filename, $nextid, $previd, $nsfw, $tmbo, $uploader, $uploaderid );
			}
		}

		filenav( $nextid, $previd, $uploaderid, $uploader );
	}

	function filenav( $nextid, $previousid, $uploader_id, $uploader_name ) {
		if( isset( $nextid ) ) {
		 ?>
			<a id="next" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $nextid ?>">newer</a>
		<? } 
		else {
			?><a href="../" id="next" style="visibility:hidden">newer</a><?
		} ?>
		 . <a id="index" href="http://themaxx.com/offensive/">index</a> .
		 <? if( isset( $previousid ) ) { ?>
			<a id="previous" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $previousid ?>">older</a>
		<? } 
	}


	function writeNavFile( $navfile, $filename, $nextid, $previd, $nsfw, $tmbo, $uploader, $uploaderid ) {
		$file = fopen( $navfile, 'w' );
		$data = '
			<!-- $navfile -->
			<?
				$filename = "' .  $filename . '";
				$nextid = "' . $nextid . '";
				$previd = "' . $previd . '";
				$nsfw = "' . $nsfw . '";
				$tmbo = "' . $tmbo . '";
				$uploader = "' . $uploader . '";
				$uploaderid = "' . $uploaderid . '";
			?>

		';
		fwrite( $file, $data );
		fclose( $file );
	}

	function getFileSize( $fname ) {
		$k = "";
		$filepath = "../images/picpile/$fname";
		if( file_exists( $filepath ) ) {
			$size = filesize( $filepath );
			$k = "(" . round( ($size/1024) ) . "k)";		
		}
		return $k;
	}

	if( ! file_exists( "../images/picpile/$filename" ) ) {
//		header("HTTP/1.0 404 Not Found");
//		include( $_SERVER['DOCUMENT_ROOT'] . "/missing.html" );
//		flush();
//		exit;
	}



?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">

<html>
	<head>
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>themaxx.com : [ this might be offensive ] : <? echo $filename ?> </title>
		<link rel="stylesheet" type="text/css" href="styles.php"/>
		<script type="text/javascript">
			self.file_id = "";
			
			// prevent sites from hosting this page in a frame;
			if( window != top ) {
				top.location.href = window.location.href;
			}
		</script>
		<script type="text/javascript" src="offensive.js"></script>

	</head>
	<body onload="doOnloadStuff()" onkeydown="return handleKeyDown( event );">
	<?php include "../message.php" ?>
	<div id="content">
		<div id="heading">

			&nbsp;&nbsp;

			<? writeNav( $id ); ?>

			 <a style="margin-left:48px;" id="comments" href="/offensive/?c=comments&fileid=<? echo $id?>">comments</a>
				<?php
					if( $_SESSION['userid'] ) {
						?>
						
						<span style="margin-left:48px;">
							vote: <a id="good"  href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20good&redirect=true">[ this is good ]</a> . 
									<a id="bad" href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20bad&redirect=true">[ this is bad ]</a>
						</span>

						<span style="margin-left:48px;">nsfw filter: <?php
							if( $_SESSION['prefs']['hide nsfw'] == 1 ) {
								?>
									<a href="/offensive/setPref.php?p=1&v=">off</a> on
								<?php
							}
							else {
								?>
									off <a href="/offensive/setPref.php?p=1&v=2">on</a>
								<?php
							}
						?></span>
						
						<span style="margin-left:48px;">tmbo filter: <?php
							if( $_SESSION['prefs']['hide tmbo'] == 1 ) {
								?>
									<a href="/offensive/setPref.php?p=3&v=">off</a> on
								<?php
							}
							else {
								?>
									off <a href="/offensive/setPref.php?p=3&v=2">on</a>
								<?php
							}
						?></span>

						<?php
						
					}
				?>

			</div>

			<br /><br />
			<? echo $nsfw == 1 ? "<span style=\"color:#990000\">[NSFW]</span>" : "" ?></span>
			<? echo $filename ?> <span style="color:#999999"><?= getFileSize( $filename ) ?></span>
			<br/>
			<span style="color:#999999">
				uploaded by <a href="../?c=user&userid=<? echo $uploaderid ?>"><? echo $uploader ?></a>
			</span>	
			<br/><br/>
			<?
				if( $_SESSION['prefs']['hide nsfw'] == 1 && $nsfw == 1 || ($_SESSION['prefs']['hide tmbo'] == 1 && $tmbo == 1) ) {
					?><div style="padding:128px;">[ filtered ]</div><?
				}
				else {
					?><div class="<?php echo $nsfw == 1 ? 'nsfw' : 'image' ?> u<?= $uploaderid ?>"><a href="../images/picpile/<? echo rawurlencode( $filename )?>" target="_blank"><img src="http://images.themaxx.com/mirror.php/offensive/images/picpile/<? echo rawurlencode( $filename )?>" style="border:none"/></a></div><?
				}
			?>
			<br/><br/>

		</div>
	</body>
</html>
