<?

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/core.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	

	function start() {
		mustLogIn();
		
		if(!isset($_REQUEST['userid']) ||
		   $_REQUEST['userid'] == "0" ||
		   !is_intger($_REQUEST['userid'])) {
			header("Location: ./");
			exit;
		}
		
		global $them;
		$them = new User($_REQUEST['userid']);
		
		if(!$them->exists()) {
			header("Location: ./");
			exit;
		}
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
			<input type="hidden" name="redirect" value="./?c=user&userid=<?= me()->id() ?>"/>
		
			<input type="file" name="image"/>
			<input type="submit" value="upload yearbook picture"/>

		</form>
	</div>
<?
	}

	function title() {
		global $them;
		return "[user] : ".$them->username();
	}
	
	function body() {
		global $them;
		if( me()->status() == "admin" && 
		    array_key_exists("set_account_status", $_REQUEST) && 
		    $_REQUEST['set_account_status'] ) {
			handleSetStatusSubmission();
		}

		$usrid = $them->id();
		$avatar = $them->yearbook();
		$name = $them->username();
		$status = $them->status();
		$email = $them->email();
		$created = $them->created();
		$referredby = $them->referred_by();
		$timestamp = $them->timestamp();
		$ip = $them->last_login_ip();
	
		switch ($status) {
			case "locked":
				$accountMessage = "$name was acting retarded and was sent home.";
			break;
			
			case "awaiting activation":
				$accountMessage = "$name has not picked up the keys yet.";
			break;

      case "memoriam":

        $accountMessage = "$name will always be missed.";

      break;
			case "admin":
			default:
				if(strtotime($them->timestamp()) < time() - 60 * 60 * 24 * 365) {
					$accountMessage = "$name gave up.";
				} else {
					$accountMessage = "$name is welcome here.";
				}
			break;
		}
		
		$formattedDate = date( "F d, Y", strtotime($created) );
		$accountDateMessage = $formattedDate == "September 15, 2004" ? "before the dawn of time" : $formattedDate;

?>
		
				<div class="contentbox">
					<div class="blackbar"></div>
					<div class="heading">
						<? echo $accountMessage ?><br/>
						<span style="color:#666699">
							<? echo "$name has been around since $accountDateMessage."?>
							<? if( $referredby ) { ?>
								(thanks to <a href="./?c=<?= $_REQUEST['c'] ?>&userid=<?= $referredby->id() ?>" style="color:#666699"><?= $referredby->username() ?></a>.)
							<? } ?>
							<? if( isset( $ip ) ) { ?>
								<br/>Last seen on <?= date( "F d, Y", strtotime($timestamp) ) ?><? if( me()->status() == "admin" ) { ?> from <?= $ip ?> (<a href="./?c=iphistory&uid=<?= $usrid ?>">view history</a>)<? } ?>.
								<?
									if( me()->status() == "admin" ) {
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
							if( me()->status() == "admin" ) {
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
								if(me()->squelched($usrid)) {
									?><a href="/offensive/setPref.php?unsq=<?= $usrid ?>">unsquelch</a><?
								} else {
									?><a href="/offensive/setPref.php?sq=<?= $usrid ?>">squelch</a><?
								}
							?>

						</div>

						<?
							if($avatar) {
// XXX: avatar history
								if($avatar->file() && !$avatar->blocked()) {
								?>
									<div class="piletitle" style="margin-bottom:12px;">
										<a href="pages/pic.php?id=<?= $avatar->id() ?>"><?= $name ?> is gorgeous.</a>
									</div>
									<!-- next onclick="img.src=arr[i]" -->
								<?
								}
							}
							
							if( me()->id() == $usrid ) {
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
									<? 
										recentComments($usrid);
										contributions($usrid)
									?>
						</table>
					</div>
					<div class="blackbar"></div>
				</div>
<?
}

function recentComments( $uid ) {
	if(me()->blocked($uid)) {
		return;
	}
	
?><tr><td>
	<div class="piletitle">the smartypants' most recent remarks:</div>
	<div style="padding:8px;"><table width="535px">
<?

	$comments = core_getcomments(array("limit" => 100, "userid" => $uid, "votefilter" => "c"));
	$seen = array();
	$count = 0;
	foreach($comments as $comment) {
		$upload = $comment->upload();
		
		// avoid duplicates
		if(array_key_exists($comment->upload()->id(), $seen) && $seen[$comment->upload()->id()] == true) {
			continue;
		}
		$seen[$comment->upload()->id()] = true;
		$count++;
		
		$css = isset($css) && strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
		?><tr class="<?= $css ?>"><td><div style="white-space:nowrap; width:535px; overflow:hidden;"><a class="<?= $css ?>" href="?c=comments&fileid=<?= $upload->id() ?>#<?= $comment->id() ?>"><?= $upload->htmlFilename() ?></a></div></td></tr><?

		if($count >= 15) {
			break;
		}
	}
?>
	</table></div></td></tr>
<?

}


function handleSetStatusSubmission() {
	$usrid =  sqlEscape( $_POST['userid'] );
	$status =  sqlEscape( $_POST['set_account_status'] );
	$email =  sqlEscape( $_POST['set_email'] );

	$them = new User($usrid);
	$content = "previously ".$them->username()."'s status was ".$them->status();

	$changed = $them->status() != $status;

	if( is_intger($usrid ) && $status !== "" ) {
		$isEmailValid = preg_match( '/[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+/', $email ) > 0;
		$emailClause = $isEmailValid ? ", email = '$email' " : "";
		$sql = "UPDATE users SET timestamp = timestamp, account_status ='$status' $emailClause WHERE userid = $usrid limit 1";
		$result = tmbo_query( $sql );

		if($changed) {
			$emails = array();
			$admins = core_getadmins();
			foreach($admins as $admin) {
				$emails[] = $admin->email();
			}
			$subject = me()->username() . " changed the status to $status for user $usrid";
			mail( implode(",", $emails), "[ tmbo ] $subject", "$content\n\nthis is an auto-generated message. replies will not be read by humans.", "From: offensive@thismight.be (this might be offensive)");
		}
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
	if(me()->blocked($id)) {
		return;
	}
	
	$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id";
	$result = tmbo_query( $sql );
	$comments = mysql_fetch_row($result);
	$comments = $comments[0];

	if($comments > 1000) {
		?><div class="piletitle" style="margin-bottom:12px;"><?echo $name ?> has made <?= $comments ?>  votes/comments<?
	} else {
		$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id AND vote = 'this is good'";
		$result = tmbo_query( $sql );
		$good = mysql_fetch_row($result);
		$good = $good[0];
		
		$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id AND vote = 'this is bad'";
		$result = tmbo_query( $sql );
		$bad = mysql_fetch_row($result);
		$bad = $bad[0];
		
		?><div class="piletitle" style="margin-bottom:12px;"><?echo $name ?>'s voting record: +<?= $good ?> -<?= $bad ?><?
	}

?> (<a href="./?c=votedetail&userid=<?= $id ?>">click for details</a>)</div>
<?

}

function contributions($usrid) {
	global $them;
	if(me()->blocked($them)) {
		return;
	}
	
	?><tr><td>
	<div class="piletitle">
		<a href="/offensive/?c=<?= (me()->getPref("index") == "thumbs") ? 
		      "thumbs" : "main" ?>&userid=<?= $usrid?>"><?= $them->username() ?>'<?= substr($them->username(), -1) == "s" ? "" : "s" ?> contributions to society:</a>
		<!-- +<?php echo isset($good_votes) ? $good_votes : 0; ?> -<?php echo isset($bad_votes) ? $bad_votes : 0; ?> (<a href="./?c=uservotedetail&userid=<?= $usrid ?>">click for details</a>)--></div>
		</td></tr>
		<tr><td valign="top">
			<div style="padding:8px;">								
				<table style="width:100%">

<?
	$uploads = core_getuploads(array("userid" => $usrid));
	$class = 'evenfile';
	foreach($uploads as $upload) {
		$css = isset($css) && strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
		?>
			<tr class="<?= $css ?>">
				<td style="width:100%"><div style="overflow:hidden;white-space:nowrap;width:285px;"><a class="<?= $css ?>" href="<?
				if($upload->type() != "topic") 
					echo "pages/pic.php?id=".$upload->id();
				else
					echo "/offensive/?c=comments&fileid=".$upload->id();
				?>"><?= $upload->htmlFilename(true) ?></div></td>
				<td class="<?php echo $css?>" style="text-align:right;white-space:nowrap;width:100%">
					<a href="./?c=comments&fileid=<?php echo $upload->id() ?>" class="<?php echo $css?>"><?=$upload->commentLabel(!($upload->type() == 'topic'));?></a>
				</td>
				<td style="text-align:right;white-space:nowrap;width:100%" class="<?php echo $css?>"><?php echo $upload->timestamp() ?></td>
			</tr>
		<?
	}
?>

				</table>
			</div>
		</td></tr><?
}

?>