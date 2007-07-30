<?php session_start();

# header( "Location: ../" );

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../logn.php?redirect=" . urlencode( $_SERVER['PHP_SELF'] . "?" . $_SERVER['QUERY_STRING'] ));
		exit;
	}

	$id = $_REQUEST['id'];
	if( ! is_numeric( $id ) ) {
		header( "Location: ../" );
	}

	$cookiename = $_SESSION['userid'] . "lastpic";

	$lastpic = $_COOKIE[ $cookiename ];
	
	if( ! is_numeric( $lastpic ) || $id > $lastpic ) {
		setcookie( $cookiename, "$id", time()+3600 * 24 * 365, "/offensive/" );
	}

	// Include, and check we've got a connection to the database.
	include_once( '../../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();
	require_once( '../getPrefs.php' );	
	
	$id = $_REQUEST['id'];
	$uid = $_REQUEST['uid'];

	function thisOrZero( $value ) {
		return (is_numeric( $value ) ? $value : 0);
	}


	function writeNav( $id ) {
	
		global $filename, $nsfw, $tmbo, $uploader, $uploaderid, $timestamp, $year, $month, $day;
		
		if( file_exists( "nav/$id.php" ) ) {
			include( "nav/$id.php" );
		}

		if( ! is_numeric( $nextid ) ) {
			$sql = "SELECT offensive_uploads.*, users.username, users.userid,
						(select min( id ) from offensive_uploads where id > $id AND type='image' and status='normal') as nextid,
						(select max( id ) from offensive_uploads where id < $id AND type='image' and status='normal') as previd
					FROM offensive_uploads, users
					WHERE id = $id 
						AND offensive_uploads.userid = users.userid
						AND type='image'
					LIMIT 1";

#						AND type='image' AND users.account_status != 'locked'
					
			$result = mysql_query( $sql );
			$row = mysql_fetch_assoc( $result );
			
			$filename = $row['filename'];
			$nextid = $row['nextid'];
			$nsfw = ( $row['nsfw'] == 1 || strpos( $filename, "nsfw" ) || strpos( $filename, "NSFW" ) );
			$previd = $row['previd'];		
			$tmbo = $row['tmbo'];
			$uploader = $row['username'];
			$uploaderid = $row['userid'];
			$time = strtotime( $row['timestamp'] );
			$year = date( "Y", $time );
			$month = date( "m", $time );			
			$day = date( "d", $time );
			$timestamp = date( "Y-m-d h:i:s a", strtotime( $row['timestamp'] ) );
			
			if( is_numeric( $nextid ) ) {
				writeNavFile( "nav/$id.php", $filename, $nextid, $previd, $nsfw, $tmbo, $uploader, $uploaderid, $timestamp, $year, $month, $day );
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
		 . <a id="index" href="/offensive/">index</a> .
		 <? if( isset( $previousid ) ) { ?>
			<a id="previous" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $previousid ?>">older</a>
		<? } 
	}


	function writeNavFile( $navfile, $filename, $nextid, $previd, $nsfw, $tmbo, $uploader, $uploaderid, $timestamp, $year, $month, $day ) {
		$escaped_fname = addslashes( $filename );
		$file = fopen( $navfile, 'w' );
		$data = '

			<?
				$filename = \'' .  $escaped_fname . '\';
				$nextid = "' . $nextid . '";
				$previd = "' . $previd . '";
				$nsfw = "' . $nsfw . '";
				$tmbo = "' . $tmbo . '";
				$uploader = "' . $uploader . '";
				$uploaderid = "' . $uploaderid . '";
				$timestamp = "' . $timestamp . '";
				$year = "' . $year . '";
				$month = "' . $month . '";
				$day = "' . $day . '";
			?>

		';
		fwrite( $file, $data );
		fclose( $file );
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
	
	$result = mysql_query( $sql );
	if( mysql_num_rows( $result ) > 0 ) {
		list( $good, $bad, $tmbo, $repost, $comments  ) = mysql_fetch_array( mysql_query( $sql ) );
	}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">

<html>
	<head>
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>[ this might be offensive ] : <? echo htmlentities($filename) ?> </title>
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
				 <a style="margin-left:48px;" id="comments" href="/offensive/?c=comments&fileid=<? echo $id?>">comments</a> (<?php echo "{$comments}c +$good -$bad"; if( $offensive > 0 ) { echo " <span style=\"color:#990000\">x$offensive</span>"; }?>)	

				<?php
					if( $_SESSION['userid'] ) {
						?>
						
						<span style="margin-left:48px;">
							vote: <a id="good"  href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20good&redirect=true">[ this is good ]</a> . 
									<a id="bad" href="/offensive/?c=comments&submit=submit&fileid=<? echo $id ?>&vote=this%20is%20bad&redirect=true">[ this is bad ]</a>
						</span>

						<span style="margin-left:48px;">
							vote: <a id="good"  href="/offensive/subscribe.php&fileid=<? echo $id ?>">subscribe</a> 
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
			<? echo $nsfw == 1 ? "<span style=\"color:#990000\">[NSFW]</span>" : "" ?></span>
			<? echo $tmbo == 1 ? "<span style=\"color:#990000\">[TMBO]</span>" : "" ?></span>
			<? echo htmlentities($filename) ?> <span style="color:#999999"><?= getFileSize( $filepath ) ?></span>
			<br/>
			<span style="color:#999999">
				uploaded by <a href="../?c=user&userid=<? echo $uploaderid ?>"><? echo $uploader ?></a> @ <?= $timestamp ?>
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
			
					if( $_SESSION['prefs']['hide nsfw'] == 1 && $nsfw == 1
						|| ($_SESSION['prefs']['hide tmbo'] == 1 && $tmbo == 1) 
						|| ( in_array( $uploaderid, explode( ',', $_SESSION['prefs']['squelched'] ) ) ) ) {
						?><div style="padding:128px;">[ filtered ] <!-- <?= $uploaderid ?> --></div><?
					}
					else {
						?>
						<div class="<?php echo $nsfw == 1 ? 'nsfw' : 'image' ?> u<?= $uploaderid ?>">
<? /*							<!-- 							<a href="<?= $filepath ?>" target="_blank"><img src="http://images.thismight.be/offensive/<?= "uploads/$year/$month/$day/image/" . rawurlencode( $imgfilename ) ?>" style="border:none"/></a>-->  */ ?>
<a href="<?= $filepath ?>" target="_blank"><img src="<?= "../uploads/$year/$month/$day/image/" . rawurlencode( $imgfilename ) ?>" style="border:none"/></a> 
						</div>
	
						<?						
					}
					
				}
			?>
			<br/><br/>
			
			<?
	
/*	
	function getOldestExistingFileId() {
		$files = loadFiles('../images/picpile/');
		SortByDate($files);
		$oldest = $files[0];
		$oldestName = mysql_escape_string( $oldest[0] );
		
		$sql = "SELECT max(id)
					FROM offensive_uploads
					WHERE type='image'
					AND filename = '$oldestName'
					AND timestamp > DATE_SUB( now(), INTERVAL 4 day )";

		list( $oldestId ) = mysql_fetch_array( mysql_query( $sql ) );
		return is_numeric( $oldestId ) ? $oldestId : -1;

	}
	
	
	function loadFiles($dir) {
		$files = array();
		$It =  opendir($dir);
		if (! $It) {
			die('Cannot list files for ' . $dir);
		}
		
		while ($filename = readdir($It))
		{
			if( $filename == '.' || $filename == '..') {
				continue;
			}
			$lastModified = filemtime($dir . $filename);
			$files[] = array($filename, $lastModified);
		}
		return $files;
	}
*/

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
