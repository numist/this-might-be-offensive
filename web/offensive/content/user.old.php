<?
	$activeTab = ($_REQUEST['userid'] == 142) ? 'meowchow' : '';
	require_once( 'tabs.php' );	

	function start() {
	
		$usrid = $_REQUEST['userid'];
		if( ! is_numeric( $usrid ) ) {
			mail( "ray@mysocalled.com", "[" . $_SERVER['REMOTE_ADDR'] . "] ATTEMPTED ATTACK?", requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			session_unset();
			header( "Location: ./" );
		}
	
	}
	
	function isAdmin() {
		$uid = $_SESSION['userid'];
		return ($uid == 151 || $uid == 200);
	}
	
	function body() {

		$usrid = $_REQUEST['userid'];

		$link = openDbConnection();
		
		
		$sql = "SELECT username, account_status, created,
					(select username from users ref where users.referred_by=ref.userid) AS referredby,
					(select userid from users ref where users.referred_by=ref.userid) AS referrerid,
					last_login_ip, timestamp
					FROM users WHERE userid={$usrid}";
		list($name, $status, $created, $referredby, $refid, $ip, $timestamp) = mysql_fetch_array(mysql_query($sql));
	
		$query_result = mysql_query('set @good=0,@bad=0');
		$sql = "SELECT max(if(vote='this is good',@good:=@good+1,@bad:=@bad+1)), max(@good), max(@bad)
			FROM offensive_comments, offensive_uploads, users
			WHERE offensive_uploads.userid ={$usrid}
			AND offensive_comments.fileid = offensive_uploads.id
			AND users.userid = offensive_uploads.userid AND vote
			";
		list($junk, $good_votes, $bad_votes) = mysql_fetch_array(mysql_query($sql));
		
		if( ! is_numeric( $good_votes ) ) {
			$good_votes = 0;
		}
	
		if( ! is_numeric( $bad_votes ) ) {
			$bad_votes = 0;
		}
	
		switch ($status) {
			case "locked":
				$accountMessage = "$name was acting retarded and was sent home.";
			break;
			
			case "awaiting activation":
				$accountMessage = "$name has not picked up his keys yet.";
			break;
			
			default:
				$accountMessage = "$name is welcome here.";
			break;
		}
		$formattedDate = date( "F d, Y", strtotime($created) );
		$accountDateMessage = $formattedDate == "September 15, 2004" ? "before the dawn of time" : $formattedDate;

?>
		

					<div class="heading">
						<? echo $accountMessage ?><br/>
						<span style="color:#666699">
							<? echo "$name has been around since $accountDateMessage."?>
							<? if( is_numeric( $refid ) && $refid != 151 ) { ?>
								(thanks to <a href="./?c=<?= $_REQUEST['c'] ?>&userid=<?= $refid?>" style="color:#666699"><?= $referredby ?></a>.)
							<? } ?>
							<? if( isset( $ip ) ) { ?>
								<br/>Last seen on <?= date( "F d, Y", strtotime($timestamp) ) ?><? if( isAdmin() ) { ?> from <?= $ip ?> (<a href="./?c=iphistory&uid=<?= $usrid ?>">view history</a>)<? } ?>.
							<? } ?>
						</span>
					</div>
					<? tabs(); ?>
					<div class="bluebox">
						
						<?
							if( file_exists( "map/users/$usrid.jpg" ) ) {
								?><div style="text-align:center;margin-bottom:12px;"><a href="map/"><img src="map/users/<?= $usrid?>.jpg" width="500" height="250"/></a></div><?
							}
						?>

						<?
							votingRecord( $usrid, $name );
						?>

						<table width="100%" border="0" cellpadding="0" cellspacing="0">

							<tr>
								<td>
									<? 
										if( $usrid == $_SESSION['userid'] ) {
											unreadComments( $usrid );
										}
										else {
											recentComments( $usrid );
										}
									?>
								</td>
							</tr>

							<tr>
								<td>
									<div class="piletitle"><?echo $name ?>'s contributions to society: +<?php echo $good_votes ?> -<?php echo $bad_votes ?> (<a href="./?c=uservotedetail&userid=<?= $usrid ?>">click for details</a>)</div>
								</td>
							</tr>
							
							<tr>
								<td valign="top">
								
<table style="width:100%">
									
<?php
		$sql = "SELECT up. * , up.id AS fileid, up.timestamp as uploadtime, users.username, comments.vote, count( comments.id ) AS thecount, sum( offensive ) AS offensive
				FROM offensive_uploads up, users
				LEFT JOIN offensive_comments comments ON up.id = comments.fileid
			WHERE up.userid = users.userid AND users.userid = $usrid AND (type='image' OR type='audio')
			GROUP BY up.id, vote
			ORDER BY up.timestamp DESC 
			LIMIT 300
";

	$result = mysql_query( $sql );

	$class = 'evenfile';
	$previousId = -1;
	$good = 0;
	$bad = 0;
	$offensive = 0;
	$comments = 0;
	$last_row_id = -1;
	while( $row = mysql_fetch_assoc( $result ) ) {
		$id = $row['fileid'];
		$vote = $row['vote'];
		if( $previousId > 0 && $previousId != $id ) {

			$count++;
			$class = ($class == 'evenfile') ? 'oddfile' : 'evenfile';
			$last_row_id = emitFileRow( $class, $previousId, $previousFilename, $comments, $good, $bad, $offensive, $previousTimestamp );
			$good = 0;
			$bad = 0;
			$comments = 0;
			$offensive = 0;
		}

		if( $vote == 'this is good' ) {
			$good = $row['thecount'];
		}
		else if( $vote == 'this is bad' ) {
			$bad = $row['thecount'];
		}
		
		$comments += $row['thecount'];
		$offensive += $row['offensive'];		
		$previousId = $id;
		$previousFilename = $row['filename'];
		$previousUsername = $row['username'];
		$previousTimestamp = $row['uploadtime'];
		
	}
	if( ($last_row_id > 0 && $last_row_id <> $previousId) || (mysql_num_rows( $result ) > 0 && $last_row_id == -1 )) {
		emitFileRow( $class, $previousId, $previousFilename, $comments, $good, $bad, $offensive, $previousTimestamp );
	}

?>
</table>

								</td>
							</tr>
						</table>
						
						
					</div>
		



<?
}

function emitFileRow( $css, $id, $filename, $comments, $good, $bad, $offensive, $timestamp ) {
?>
	<tr>
		<td style="width:100%" class="<?php echo $class?>"><div class="clipper"><a href="pages/pic.php?id=<?php echo $id ?>" class="<?php echo $css?>"><?= maxString($filename,45) ?></div></td>
		<td class="<?php echo $class?>" style="text-align:right;white-space:nowrap;width:100%"><a href="./?c=comments&fileid=<?php echo $id ?>" class="<?php echo $css?>"><? echo "$comments comments (+$good -$bad x$offensive)" ?></a></td>
		<td style="text-align:right;white-space:nowrap;width:100%" class="<?php echo $css?>"><?php echo $timestamp?></td>
	</tr>
<?
	return $id;
}


function unreadComments( $uid ) {

	$currentUserId = $_SESSION['userid'];
	if( $currentUserId != $uid ) {
		return;
	}
	
	
	$sql = "select distinct fileid, filename
			from offensive_uploads u, offensive_bookmarks b
			where b.userid=$uid
			and u.id = b.fileid
			order by b.timestamp desc
			LIMIT 30
		";

	$result = mysql_query( $sql );
	
	if( mysql_num_rows( $result ) == 0 ) {
		return;
	}

?>

	<div class="piletitle">there are comments you haven't seen:</div>
	<div style="padding:8px;">

<?

	while( $row = mysql_fetch_assoc( $result ) ) {
		$css = $css == "evenfile" ? "oddfile" : "evenfile";
?>
		<div><a class="<?= $css ?>" href="?c=comments&fileid=<?= $row['fileid'] ?>"><?= $row['filename']?></a></div>
<?

	}
?>
	</div>
<?

}

function recentComments( $uid ) {

	$sql="SELECT up.id, up.filename, offensive_comments.timestamp, offensive_comments.id as commentid
			FROM offensive_uploads up, offensive_comments
			WHERE offensive_comments.userid = $uid
			AND up.id = offensive_comments.fileid
			AND comment <> \"\"
			GROUP BY fileid
			ORDER  BY offensive_comments.timestamp DESC 
			LIMIT 15";

	$link = openDbConnection();
	$result = mysql_query( $sql );
	
?>
	<div class="piletitle">the smartypants' most recent remarks:</div>
	<div style="padding:8px;">

<?

	while( $row = mysql_fetch_assoc( $result ) ) {
		$css = $css == "evenfile" ? "oddfile" : "evenfile";
?>
		<div><a class="<?= $css ?>" href="?c=comments&fileid=<?= $row['id'] ?>#<?= $row['commentid'] ?>"><?= $row['filename']?></a></div>
<?
	}
?>
	</div>
<?

}


function votingRecord( $id, $name ) {
	
	$sql = "SELECT count( vote ) as count, vote
				FROM offensive_comments
				WHERE userid = $id
				GROUP  BY vote";

	$link = openDbConnection();
	
	$result = mysql_query( $sql );

	$good = 0;
	$bad = 0;
	$comments = 0;
	
	while( $row = mysql_fetch_array( $result ) ) {
	
		switch( $row['vote'] ) {
		
			case 'this is good':
				$good = $row['count'];
			break;
			
			case 'this is bad':
				$bad = $row['count'];
			break;
			
			default:
				$comments = $row['count'];
			break;
		
		}

	}

?>
	<div class="piletitle" style="margin-bottom:12px;"><?echo $name ?>'s voting record: <? echo "+$good -$bad"?> (<a href="./?c=votedetail&userid=<?= $id ?>">click for details</a>)</div>

<?

}

?>
