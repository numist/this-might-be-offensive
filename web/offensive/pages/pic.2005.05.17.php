<?php session_start();

	header( "Location: ../index.outoforder.php" );

	$id = $_REQUEST['id'];
	
	$lastpic = $_COOKIE['lastpic'];
	
	if( ! is_numeric( $lastpic ) || $id > $lastpic ) {
		setcookie( 'lastpic', "$id", time()+3600 * 24 * 365, "/offensive/" );
	}

	require_once( '../../admin/mysqlConnectionInfo.php' );
	$link = openDbConnection();
	
	$id = $_REQUEST['id'];
	$uid = $_REQUEST['uid'];

	$navFile = "./nav/${id}.php";

	if( ! file_exists( $navFile ) ) {
		writeNav( $id, $uid, $navFile );
	}

	function writeNav( $id, $uid, $navFile ) {	

		global $next, $previous, $next_nsfw, $prev_nsfw, $uid, $uploader, $uploaderid;

		$sql = "SELECT offensive_uploads.*, users.username, users.userid from offensive_uploads, users
					WHERE id >= $id AND offensive_uploads.userid = users.userid AND type='image' AND users.account_status != 'locked'
					UNION (select offensive_uploads.*, users.userid, users.username from offensive_uploads, users
					WHERE id < $id AND offensive_uploads.userid = users.userid AND type='image' AND users.account_status != 'locked' 
					order by id desc limit 1)
				ORDER BY id LIMIT 3";
	
		$result = mysql_query( $sql );
		while( $row = mysql_fetch_assoc( $result ) ) {
			global $previous, $next, $filename, $uploader, $uploaderid, $nsfw, $next_nsfw, $prev_nsfw;
			$temp = $row['id'];
			if( $temp < $id ) {
				$previous = $temp;
				$prev_nsfw = ($row['nsfw'] == 1 || strpos( strtolower($filename), "nsfw" ) > -1 ) ? 1 : 0;			
			}
			else if( $temp > $id ) {
				$next = $temp;
				$next_nsfw = ($row['nsfw'] == 1 || strpos( strtolower($filename), "nsfw" ) > -1 ) ? 1 : 0;
			}
			else if( $temp == $id ) {
				$filename = $row['filename'];
				$uploaderid = $row['userid'];
				$uploader = $row['username'];
				$nsfw = ($row['nsfw'] == 1 || strpos( strtolower($filename), "nsfw" ) > -1 ) ? 1 : 0;
				$tmbo = ($row['tmbo'] == 1) ? 1 : 0;			
			}
		}

		if( isset( $next ) && is_numeric( $uploaderid ) && is_numeric( $nsfw ) ) {
			ob_start();
			filenav( true );
			$nav = ob_get_contents();
			ob_end_clean();
			$FILE = fopen( $navFile, 'w' );
			fwrite( $FILE, $nav );
			fwrite( $FILE, '<? $uploader = \'' . $uploader . '\';
								$filename = \'' . str_replace( '\'', '\\\'', $filename ) . '\';
								$nsfw = ' . $nsfw . ';
								$uploaderid = ' . $uploaderid . ';

							?>' );
			fclose( $FILE );
		}

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
					sum( repost ) as repost
				FROM offensive_comments c
				WHERE c.fileid = $id AND c.vote
				GROUP  BY c.vote
			UNION 
				(SELECT count( cm.id ) AS votecount, 'comments', 0, 0
					FROM offensive_comments cm
					WHERE cm.fileid = $id AND cm.comment <> ''
			GROUP BY fileid)";

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
			break;
			
			case 'this is bad':
				$bad = $row['votecount'];
			break;
			
			case '':
				$commentcount = $row['votecount'];
			break;
		}
		
	}
	
	function filenav( $include_ids ) {
		
		global $next, $previous, $next_nsfw, $prev_nsfw, $uid, $uploader, $uploaderid;
		
		$userparam = is_numeric( $uid ) ? "&uid=$uid" : "";
		
		 if( isset( $next) ) {
		 	$style = $next_nsfw ? "color:#990000" : "";
		 ?>
			<a <? if( $include_ids ) {?>id="next" <?}?> href="<? echo $_SERVER['PHP_SELF']?>?id=<? echo $next . $userparam?>" style="<? echo $style ?>">newer</a>
		<? } 
		else {
			?><a href="../" id="next" style="visibility:hidden">newer</a><?
		} ?>
		 . <a id="index" href="http://themaxx.com/offensive/">index</a> .
		 <? if( isset( $previous) ) {
 		 	$style = $prev_nsfw ? "color:#990000" : "";
 		 ?>
			<a <? if( $include_ids ) {?>id="previous" <?}?> href="<? echo $_SERVER['PHP_SELF']?>?id=<? echo $previous . $userparam?>" style="<? echo $style ?>">older</a>
		<? } 
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

			<? 
				if( file_exists( $navFile ) ) {
					include( $navFile );
				}
				else {
					filenav( true );
				}
			?>

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
			<? echo $filename ?>
			<br/>
			<span style="color:#999999">
			<?
				if( is_numeric( $uid ) ) {
			?>	
				displaying uploads from <a href="../?c=user&userid=<? echo $uploaderid ?>"><? echo $uploader ?></a>.
				<a style="margin-left:32px" href="<? echo $_SERVER['PHP_SELF']?>?id=<? echo $id ?>">show uploads from all users</a>
			<?
			}
			else {
			?>
				uploaded by <a href="../?c=user&userid=<? echo $uploaderid ?>"><? echo $uploader ?></a>
			<?
			}
			?>
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
