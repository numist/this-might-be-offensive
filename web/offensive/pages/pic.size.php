<?php session_start();

	$id = $_REQUEST['id'];
	
	$lastpic = $_COOKIE['lastpic'];
	
	if( ! is_numeric( $lastpic ) || $id > $lastpic ) {
		setcookie( 'lastpic', "$id", time()+3600 * 24 * 365, "/offensive/" );
	}

	require_once( '../../admin/mysqlConnectionInfo.php' );
	$link = openDbConnection();
	
	$id = $_REQUEST['id'];
	$uid = $_REQUEST['uid'];

	function writeNav( $id, $uid ) {
	
		global $uploader, $uploaderid, $filename, $nsfw, $tmbo;

		$sql = "SELECT offensive_uploads.*, users.username, users.userid,
					(select min( id ) from offensive_uploads where id > $id AND type='image') as nextid,
					(select max( id ) from offensive_uploads where id < $id AND type='image') as previd
				FROM offensive_uploads, users
				WHERE id = $id 
					AND offensive_uploads.userid = users.userid
					AND type='image' AND users.account_status != 'locked'
				LIMIT 1";
				
		$result = mysql_query( $sql );
		$row = mysql_fetch_assoc( $result );
		filenav( $row['nextid'], $row['previd'], $uid, $row['userid'], $row['username'] );
		
		$uploaderid = $row['userid'];
		$uploader = $row['username'];
		$filename = $row['filename'];
		$nsfw = $row['nsfw'];
		$tmbo = $row['tmbo'];
	}
	
	function filenav( $nextid, $previousid, $uid, $uploader_id, $uploader_name ) {
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

	function getFileSize( $fname ) {
		$k = "";
		$filepath = "../images/picpile/$fname";
		if( file_exists( $filepath ) ) {
			$size = filesize( $filepath );
			$k = "(" . round( ($size/1024), 1 ) . "k)";		
		}
		return $k;
	}

	if( ! file_exists( "../images/picpile/$filename" ) ) {
//		header("HTTP/1.0 404 Not Found");
//		include( $_SERVER['DOCUMENT_ROOT'] . "/missing.html" );
//		flush();
//		exit;
	}


	$sql = "SELECT count( c.id ) as votecount,
					c.vote,
					sum( offensive ) as offensive,
					sum( repost ) as repost,
					(SELECT count( cm.id ) from offensive_comments cm
						WHERE cm.fileid = $id AND cm.comment <> '' LIMIT 1) AS comments
				FROM offensive_comments c
				WHERE c.fileid = $id AND c.vote
				GROUP  BY c.vote";

	$result = mysql_query( $sql );
	
	$offensive = 0;
	$good = 0;
	$bad = 0;
	$repost = 0;
	$totalresponses = 0;
	$commentcount = 0;	
	
	while( $row = mysql_fetch_assoc( $result ) ) {
		
		$offensive += $row['offensive'];
		$repost += $row['repost'];
		$totalresponses += $row['votecount'];
		
		switch( $row['vote'] ) {
			case 'this is good':
				$good = $row['votecount'];
				$commentcount = $row['comments'];				
			break;
			
			case 'this is bad':
				$bad = $row['votecount'];
				$commentcount = $row['comments'];				
			break;
			
			case '':
				$commentcount = $row['comments'];
			break;
		}

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

			<? writeNav( $id, $uid ); ?>

			 <a style="margin-left:48px;" id="comments" href="/offensive/?c=comments&fileid=<? echo $id?>">comments</a> (<?php echo "{$commentcount}c +$good -$bad"; if( $offensive > 0 ) { echo " <span style=\"color:#990000\">x$offensive</span>"; }?>)	
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
					?><div class="<?php echo $nsfw == 1 ? 'nsfw' : 'image' ?>"><a href="../images/picpile/<? echo rawurlencode( $filename )?>" target="_blank"><img src="http://images.themaxx.com/mirror.php/offensive/images/picpile/<? echo rawurlencode( $filename )?>" style="border:none"/></a></div><?
				}
			?>
			<br/><br/>

		</div>
	</body>
</html>
