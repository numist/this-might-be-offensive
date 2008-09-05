<?

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/getPrefs.inc' );
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	

	function start() {
		mustLogIn();
		
		if(!isset($_REQUEST['userid']) || $_REQUEST['userid'] == "0") { header("Location: ./"); }
	}
	
	function isAdmin() {
		$status = $_SESSION['status'];
		return $status == 'admin';
	}
	
	function uploadAvatarForm() {
?>
	<div style="margin-bottom:12px">
		<form method="post"
				action="<?php echo $_SERVER['PHP_SELF']?>"
				enctype="multipart/form-data"
				onsubmit="return (this.elements['image'].value != '')">

			<input type="hidden" name="avatar" value="1"/>
			<input type="hidden" name="c" value="upload"/>
			<input type="hidden" name="redirect" value="./?c=user&userid=<?= $_SESSION['userid']?>"/>
		
			<input type="file" name="image"/>
			<input type="submit" value="upload yearbook picture"/>

		</form>
	</div>
<?
	}
	
	function body() {
		if( isAdmin() && 
		    array_key_exists("set_account_status", $_REQUEST) && 
		    $_REQUEST['set_account_status'] ) {
			handleSetStatusSubmission();
		}

		$usrid = $_REQUEST['userid'];
		
		$sql = "SELECT username, account_status, email, created,
					(select username from users ref where users.referred_by=ref.userid) AS referredby,
					(select userid from users ref where users.referred_by=ref.userid) AS referrerid,
					last_login_ip, timestamp
					FROM users WHERE userid={$usrid}";
		$res = tmbo_query( $sql );
		list($name, $status, $email, $created, $referredby, $refid, $ip, $timestamp) = mysql_fetch_array( $res );
	
		$sql = "SELECT id, filename, timestamp FROM offensive_uploads
					WHERE userid={$usrid}
						AND type='avatar'
					ORDER BY TIMESTAMP DESC
					LIMIT 1";

		$result = tmbo_query( $sql );
		if( mysql_num_rows( $result ) == 1 ) {
			list($avid, $avatar, $avstamp) = mysql_fetch_array( $result );
		}
	
		switch ($status) {
			case "locked":
				$accountMessage = "$name was acting retarded and was sent home.";
			break;
			
			case "awaiting activation":
				$accountMessage = "$name has not picked up the keys yet.";
			break;
		
			case "admin":
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
							<? if( is_numeric( $refid ) && $refid != $usrid ) { ?>
								(thanks to <a href="./?c=<?= $_REQUEST['c'] ?>&userid=<?= $refid?>" style="color:#666699"><?= $referredby ?></a>.)
							<? } ?>
							<? if( isset( $ip ) ) { ?>
								<br/>Last seen on <?= date( "F d, Y", strtotime($timestamp) ) ?><? if( isAdmin() ) { ?> from <?= $ip ?> (<a href="./?c=iphistory&uid=<?= $usrid ?>">view history</a>)<? } ?>.
								<?
									if( isAdmin() ) {
										$sql = "SELECT DISTINCT userid, username, account_status FROM users WHERE last_login_ip = '$ip' AND userid != $usrid" ;
										$result = tmbo_query( $sql );
										if( mysql_num_rows( $result ) > 0 ) {
											?><br/>Other users with the same ip:<br/><?
											while( $row = mysql_fetch_assoc( $result ) ) {
												?>
													<a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?></a> ( <?= $row['account_status']?> )
													<br/>
												<?
											}
										}
									}
								?>
							<? } ?>
						</span>
					</div>
					<? tabs(); ?>
					<div class="bluebox">
						
						<?
							if( isAdmin() ) {
								?>
								<div class="adminControls"><?
									setStatusForm( $usrid, $status, $email );
								?>
									<a href="mailto:<?= $email ?>"><?= $email ?></a>
								</div>
								<?
							}
						?>
						
						<div style="float:right">
							<?
								if( isSquelched( $usrid ) ) {
									?><a href="/offensive/setPref.php?unsq=<?= $usrid ?>">unsquelch</a><?
								}
								else {
									?><a href="/offensive/setPref.php?sq=<?= $usrid ?>">squelch</a><?
								}
							?>

						</div>

						<?
							if(isset($avstamp) && isset($avatar) && isset($avid)) {
// XXX: avatar history
								if(getFile($avid, $avatar, $avstamp, 'avatar')) {
								?>
									<div class="piletitle" style="margin-bottom:12px;">
										<a href="<?= getFileURL($avid, $avatar, $avstamp, 'avatar') ?>" target="_blank"><?= $name ?> is gorgeous.</a>
									</div>
									<!-- next onclick="img.src=arr[i]" -->
								<?
								}
							}
							
							if( $_SESSION['userid'] == $usrid ) {
								uploadAvatarForm();
							}
							
							
							$sql = "SELECT count(*) as thecount FROM users
										WHERE referred_by=$usrid AND userid != $usrid";
							$res = tmbo_query( $sql );
							list( $possecount ) = mysql_fetch_array( $res );
							
							if( $possecount > 0 ) {
							
						?>
								<div class="piletitle" style="margin-bottom:12px;">
									<a href="./?c=posse&userid=<?=$usrid?>"><?= $name ?> has a posse.</a>
								</div>

						<?
							}
						?><div style="clear:both"></div><?

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
										recentComments( $usrid );
									?>
								</td>
							</tr>
							<tr>
								<td>
									<div class="piletitle"><a href="/offensive/?c=thumbs&userid=<?php echo $usrid?>"><?echo $name ?>'s contributions to society:</a><!-- +<?php echo isset($good_votes) ? $good_votes : 0; ?> -<?php echo isset($bad_votes) ? $bad_votes : 0; ?> (<a href="./?c=uservotedetail&userid=<?= $usrid ?>">click for details</a>)--></div>
								</td>
							</tr>
						
							<tr>
								<td valign="top">
<div style="padding:8px;">								
<table style="width:100%">
									
<?php
		$sql = "SELECT up.*, up.id AS fileid, up.timestamp as uploadtime, counts.*
				FROM offensive_uploads up
				LEFT JOIN offensive_count_cache counts ON up.id = counts.threadid
			WHERE up.userid = $usrid AND status != 'pending'
			ORDER BY up.timestamp DESC 
			LIMIT 300
";

	$result = tmbo_query( $sql );

	$class = 'evenfile';

	while( $row = mysql_fetch_assoc( $result ) ) {
		$id = $row['fileid'];
		$css = isset($css) && strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
		emitFileRow( $css, $row );
	}

?>
</table>
</div>

								</td>
							</tr>
						</table>
					</div>
<?
}

function emitFileRow( $css, $row ) {
	$id = $row['fileid'];
	$filename = $row['filename'];
	$comments = $row['comments'];
	$good = $row['good'];
	$bad = $row['bad'];
	$offensive = $row['tmbo'];
	$nsfw = $row['nsfw'];
	$timestamp = $row['uploadtime'];
	$type = $row['type'];
?>
	<tr class="<?= $css ?>">
		<td style="width:100%"><div style="overflow:hidden;white-space:nowrap;width:285px;"><a class="<?= $css ?>" href="<?
		if($type == "image" || $type == "avatar") 
			echo "pages/pic.php?id=$id";
		else
			echo "/offensive/?c=comments&fileid=$id";
		?>"><?= htmlFilename($row, true) ?></div></td>
		<td class="<?php echo $css?>" style="text-align:right;white-space:nowrap;width:100%"><a href="./?c=comments&fileid=<?php echo $id ?>" class="<?php echo $css?>"><?=
			commentLabel($comments, $good, $bad, $offensive, !($type == 'topic'));
		?></a></td>
		<td style="text-align:right;white-space:nowrap;width:100%" class="<?php echo $css?>"><?php echo $timestamp?></td>
	</tr>
<?
	return $id;
}

function recentComments( $uid ) {
	$sql="SELECT up.*, offensive_comments.timestamp, offensive_comments.id as commentid
			FROM offensive_uploads up, offensive_comments
			WHERE offensive_comments.userid = $uid
			AND up.id = offensive_comments.fileid
			AND comment <> \"\"
			GROUP BY fileid
			ORDER  BY offensive_comments.timestamp DESC 
			LIMIT 15";
	$result = tmbo_query( $sql );
	
?>
	<div class="piletitle">the smartypants' most recent remarks:</div>
	<div style="padding:8px;"><table width="535px">

<?

	while( $row = mysql_fetch_assoc( $result ) ) {
		$css = isset($css) && strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
?>
		<tr class="<?= $css ?>"><td><div style="white-space:nowrap; width:535px; overflow:hidden;"><a class="<?= $css ?>" href="?c=comments&fileid=<?= $row['id'] ?>#<?= $row['commentid'] ?>"><?= htmlFilename($row) ?></a></div></td></tr>
<?
	}
?>
	</table></div>
<?

}


function handleSetStatusSubmission() {
	$usrid =  sqlEscape( $_POST['userid'] );
	$status =  sqlEscape( $_POST['set_account_status'] );
	$email =  sqlEscape( $_POST['set_email'] );
	if( is_numeric($usrid ) && $status !== "" ) {
		$isEmailValid = preg_match( '/[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+/', $email ) > 0;
		$emailClause = $isEmailValid ? ", email = '$email' " : "";
		$sql = "update users set account_status ='$status' $emailClause where userid = $usrid limit 1";
		$result = tmbo_query( $sql );
	}
}

function setStatusForm( $id, $status, $email ) {

	$sql = "SHOW COLUMNS FROM users LIKE 'account_status'";
	$result = tmbo_query( $sql );
	$row = mysql_fetch_row($result);
	$options = explode("','",preg_replace("/(enum|set)\('(.+?)'\)/","\\2",$row[1]));
	
	?>
		<div style="margin:8px 0px;">
			<form action="<?= $_SERVER['PHP_SELF']?>" method="post">
				<input type="hidden" name="c" value="user"/>
				<input type="hidden" name="userid" value="<?= $id ?>"/>
				<select name="set_account_status">
				<?
				foreach( $options as $option ) {
					$selected = ($option == $status ) ? "selected=\"selected\"" : "";
					?>		
						<option value="<?=$option?>" <?= $selected ?>><?=$option?></option>
					<?
				}
				?>
				</select>
				<input type="text" name="set_email" value="<?= $email ?>"/>
				<input type="submit" value="update account"/>
			</form>
		</div>
	<?

}


function votingRecord( $id, $name ) {
	//return;
	
// XXX: this query needs an index to be created in the database
	$sql = "SELECT count(*) as count, vote
				FROM offensive_comments
				WHERE userid = $id
				GROUP  BY vote";

	$result = tmbo_query( $sql );

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
