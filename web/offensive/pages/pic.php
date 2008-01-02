<?php 

	set_include_path("../..");
	require_once("offensive/assets/header.inc");

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../logn.php?redirect=" . urlencode( $_SERVER['PHP_SELF'] . "?" . $_SERVER['QUERY_STRING'] ));
		exit;
	}

	$id = $_REQUEST['id'];
	if( ! is_numeric( $id ) ) {
		header( "Location: ../" );
	}

	$cookiename = $_SESSION['userid'] . "lastpic";

	$lastpic = array_key_exists($cookiename, $_COOKIE) ? $_COOKIE[ $cookiename ] : "";
	
	if( ! is_numeric( $lastpic ) || $id > $lastpic ) {
		setcookie( $cookiename, "$id", time()+3600 * 24 * 365, "/offensive/" );
	}

	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/getPrefs.inc');
	require_once('offensive/assets/functions.inc');
	
	$id = $_REQUEST['id'];

	function thisOrZero( $value ) {
		return (is_numeric( $value ) ? $value : 0);
	}


	function writeNav( $id ) {
	
		global $filename, $is_nsfw, $is_tmbo, $uploader, $uploaderid, $timestamp, $year, $month, $day;
		
		$sql = "SELECT offensive_uploads.*, users.username, users.userid,
					(select min( id ) from offensive_uploads where id > $id AND type='image' and status='normal') as nextid,
					(select max( id ) from offensive_uploads where id < $id AND type='image' and status='normal') as previd
				FROM offensive_uploads, users
				WHERE id = $id 
					AND offensive_uploads.userid = users.userid
					AND type='image'
				LIMIT 1";
#					AND type='image' AND users.account_status != 'locked'
					
		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		$row = mysql_fetch_assoc( $result );
			
		$filename = $row['filename'];
		$nextid = $row['nextid'];
		$is_nsfw = ( $row['nsfw'] == 1 || strpos( $filename, "nsfw" ) || strpos( $filename, "NSFW" ) );
		$previd = $row['previd'];		
		$is_tmbo = $row['tmbo'];
		$uploader = $row['username'];
		$uploaderid = $row['userid'];
		$time = strtotime( $row['timestamp'] );
		$year = date( "Y", $time );
		$month = date( "m", $time );			
		$day = date( "d", $time );
		$timestamp = date( "Y-m-d h:i:s a", strtotime( $row['timestamp'] ) );
			

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
		 . <a id="index" href="/offensive/">index</a> .
		 <? if( isset( $previousid ) ) { ?>
			<a id="previous" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $previousid ?>">older</a>
		<? } 
	}

	function getFileSize( $fpath ) {
		$k = "";
		if( file_exists( $fpath ) ) {
			$size = filesize( $fpath );
			$k = "(" . round( ($size/1024) ) . "k)";		
		}
		return $k;
	}

	$good = 0;
	$bad = 0;
	$tmbo = 0;
	$repost = 0;
	$comments = 0;

	$sql = "SELECT good, bad, tmbo, repost, comments from offensive_count_cache c
			WHERE threadid=$id";
	
	$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	if( mysql_num_rows( $result ) > 0 ) {
		list( $good, $bad, $tmbo, $repost, $comments  ) = mysql_fetch_array( $result );
	}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">

<html>
	<head>
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>[ this might be offensive ] : <? echo isset($filename) ? $filename : ""; ?> </title>
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
	<!-- message -->
	<div style="white-space:nowrap;overflow:hidden;padding:3px;margin-bottom:0px;background:#000033;color:#ff6600;font-size:10px;font-weight:bold;padding-left:4px;">
		<div style="float:right;"><a href="#" style="color:#ff6600" onclick="toggleVisibility( document.getElementById( 'instructions' ) ); return false">?</a></div>
		<div>consciousness doesn't really exist. it's just another one of our ideas.</div>
	</div>
	<div id="instructions" style="display:none;white-space:nowrap;overflow:hidden;padding:3px;margin-bottom:6px;background:#cccccc;color:#333333">left arrow = newer . up arrow = index . right arrow = older . down arrow = comments . plus key = [ this is good ] . minus key = [ this is bad ] . (because clicking is too hard.)</div>

	<div id="content">
		<div id="heading">

			&nbsp;&nbsp;

				<? writeNav( $id ); ?>
				 <a style="margin-left:48px;" id="comments" href="/offensive/?c=comments&fileid=<? echo $id?>">comments</a> (<?php echo "{$comments}c +$good -$bad"; if( $tmbo > 0 ) { echo " <span style=\"color:#990000\">x$tmbo</span>"; }?>)	

				<?php
					if( $_SESSION['userid'] ) {
						?>
						
						<span style="margin-left:48px;">
							vote: <a id="good"  href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20good&redirect=true">[ this is good ]</a> . 
									<a id="bad" href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20bad&redirect=true">[ this is bad ]</a>
						</span>

						<span style="margin-left:48px;">
							<a href="/offensive/subscribe.php?fileid=<? echo $id ?>">subscribe</a> 
						</span>

						<span style="margin-left:48px;">nsfw filter: <?php
							if( array_key_exists("prefs", $_SESSION) &&
							    is_array($_SESSION['prefs']) &&
							    array_key_exists("hide nsfw", $_SESSION['prefs']) &&
							    $_SESSION['prefs']['hide nsfw'] == 1 ) {
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
							if( array_key_exists("prefs", $_SESSION) &&
							    is_array($_SESSION['prefs']) &&
							    array_key_exists("hide tmbo", $_SESSION) &&
							    $_SESSION['prefs']['hide tmbo'] == 1 ) {
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

			<?
				$extension = substr( $filename, strrpos( $filename, '.' ) );
				$filepath = "../uploads/$year/$month/$day/image/$id$extension";
				$imgfilename = "$id$extension";
				if( ! file_exists( $filepath ) ) {
					$filepath = "../uploads/$year/$month/$day/image/$filename";
					$imgfilename = "$filename";
				}

			?>

			<br /><br />
			<? echo $is_nsfw == 1 ? "<span style=\"color:#990000\">[NSFW]</span>" : "" ?></span>
			<? echo $is_tmbo == 1 ? "<span style=\"color:#990000\">[TMBO]</span>" : "" ?></span>
			<? echo htmlEscape($filename); ?> <span style="color:#999999"><?= getFileSize( $filepath ) ?></span>
			<br/>
			<span style="color:#999999">
				uploaded by <a href="../?c=user&userid=<? echo $uploaderid ?>"><? echo htmlEscape($uploader); ?></a> @ <?= $timestamp ?>
			</span>	
			<span style="margin-left:48px">
				<?
				if( isSquelched( $uploaderid ) ) {
					?><a style="color:#999999" href="/offensive/setPref.php?unsq=<?= $uploaderid ?>">unsquelch <?= $uploader ?></a><?
				}
				else {
					?><a style="color:#999999" href="/offensive/setPref.php?sq=<?= $uploaderid ?>">squelch <?= $uploader ?></a><?
				}
				?>
			</span>
			<br/><br/>
			<?
				if( ! file_exists( $filepath ) ) {
#					$oldId = getOldestExistingFileId();
					if( $oldId > 0 ) {
						?>
							<div style="padding:128px;">
								<p>This image is unavailable and has probably expired.</p>
							<!--	<p><a href="./pic.php?id=<?=$oldId?>">Click here</a> to jump to the oldest unexpired image.</p> -->
							</div>
						<?
					}
				}
				else {
			
					if( array_key_exists("prefs", $_SESSION) &&
					    is_array($_SESSION['prefs']) &&
					    (array_key_exists("hide nsfw", $_SESSION['prefs']) && 
					    $_SESSION['prefs']['hide nsfw'] == 1 && $is_nsfw == 1) || 
					    (array_key_exists("hide tmbo", $_SESSION['prefs']) &&
					    $_SESSION['prefs']['hide tmbo'] == 1 && $is_tmbo == 1) 
						|| ( in_array( $uploaderid, explode( ',', $_SESSION['prefs']['squelched'] ) ) ) ) {
						?><div style="padding:128px;">[ filtered ] <!-- <?= $uploaderid ?> --></div><?
					}
					else {
						?>
						<div class="<?php echo $is_nsfw == 1 ? 'nsfw' : 'image' ?> u<?= $uploaderid ?>">
<? /*							<!-- 							<a href="<?= $filepath ?>" target="_blank"><img src="http://images.thismight.be/offensive/<?= "uploads/$year/$month/$day/image/" . rawurlencode( $imgfilename ) ?>" style="border:none"/></a>-->  */ ?>
<a href="<?= $filepath ?>" target="_blank"><img src="<?= "../uploads/$year/$month/$day/image/" . rawurlencode( $imgfilename ) ?>" style="border:none"/></a> 
						</div>
	
						<?						
					}
					
				}
			?>
			<br/><br/>
			
			<?
	
	function DateCmp($a, $b) {
		return ($a[1] < $b[1]) ? -1 : 1;
	}

	function SortByDate(&$files) {
		usort($files, 'DateCmp');
	}

	
	
	
	
	
	
	
	
	

			?>

		</div>
	</body>
</html>
