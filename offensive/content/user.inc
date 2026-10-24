<?php

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
			<input type="hidden" name="redirect" value="<?= Link::user(me()) ?>"/>
		
			<input type="file" name="image"/>
			<input type="submit" value="upload yearbook picture"/>

		</form>
	</div>
<?php
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
				$accountMessage = "$name flamed out and was put down for a nap.";
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
						<?php echo $accountMessage ?><br/>
						<span style="color:var(--link-user)">
							<?php echo "$name has been around since $accountDateMessage."?>
							<?php if( $referredby ) { ?>
								(thanks to <?= $referredby->htmlUsername() ?>.)
							<?php } ?>
							<?php if( isset( $ip ) ) { ?>
								<br/>Last seen on <?= date( "F d, Y", strtotime($timestamp) ) ?><?php if( me()->status() == "admin" ) { ?> from <?= $ip ?> (<a href="<?= Link::content("iphistory") ?>&uid=<?= $usrid ?>">view history</a>)<?php } ?>.
								<?php
									if( me()->status() == "admin" ) {
										$sql = "SELECT DISTINCT * FROM users WHERE last_login_ip = '$ip' AND userid != $usrid" ;
										$result = tmbo_query( $sql );
										if( mysqli_num_rows( $result ) > 0 ) {
											?><br/>Other users with the same ip:<br/><?php
											while( $row = mysqli_fetch_assoc( $result ) ) {
												echo id(new User($row))->htmlUsername()."<br/>";
											}
										}
									}
								?>
							<?php } ?>
						</span>
					</div>
					<?php tabs(); ?>
					<div class="bluebox">
						
						<?php
							if( me()->status() == "admin" ) {
								?>
								<div class="adminControls"><?php
									setStatusForm( $usrid, $status, $email );
								?>
									<a href="mailto:<?= htmlEscape($email) ?>"><?= htmlEscape($email) ?></a>
								</div>
								<?php
							}
						?>
						
						<div style="float:right">
							<?php
								if(me()->squelched($usrid)) {
									?><a href="/offensive/setPref.php?unsq=<?= $usrid ?>">unsquelch</a><?php
								} else {
									?><a href="/offensive/setPref.php?sq=<?= $usrid ?>">squelch</a><?php
								}
							?>

						</div>

						<?php
							if($avatar) {
// XXX: avatar history
								if($avatar->file() && !$avatar->squelched()) {
								?>
									<div class="piletitle" style="margin-bottom:12px;">
										<a href="<?= Link::upload($avatar) ?>"><?= $name ?> is gorgeous.</a>
									</div>
									<!-- next onclick="img.src=arr[i]" -->
								<?php
								}
							}
							
							if( me()->id() == $usrid ) {
								uploadAvatarForm();
							}
							
							
							$sql = "SELECT count(*) as thecount FROM users
										WHERE referred_by=$usrid AND userid != $usrid";
							$res = tmbo_query( $sql );
							list( $possecount ) = mysqli_fetch_array( $res );
							
							if( $possecount > 0 ) {
							
						?>
								<div class="piletitle" style="margin-bottom:12px;">
									<a href="<?= Link::content("posse")."&userid=".$usrid ?>"><?= $name ?> has a posse.</a>
								</div>

						<?php
							}
						?><div style="clear:both"></div><?php

							// XXX: wow this is old
							if( file_exists( "map/users/$usrid.jpg" ) ) {
								?><div style="text-align:center;margin-bottom:12px;"><a href="map/"><img src="map/users/<?= $usrid?>.jpg" width="500" height="250"/></a></div><?php
							}
						?>

						<?php
							votingRecord( $usrid, $name );
						?>

						<?php
							recentComments($usrid);
							contributions($usrid);
						?>
					</div>
					<div class="blackbar"></div>
				</div>
<?php
}

function recentComments( $uid ) {
	if(me()->squelched($uid)) {
		return;
	}
	
?><h3>the smartypants' most recent remarks:</h3>
	<ul class="imagelist">
<?php

	// TODO: THIS IS REALLY SLOW
	//$comments = core_getcomments(array("limit" => 100, "userid" => $uid, "votefilter" => "c", "sort" => "date_desc"));
	
	$sql="SELECT up.*, offensive_comments.id as commentid
	      FROM offensive_uploads up, offensive_comments
	      WHERE offensive_comments.userid = $uid
	      AND up.id = offensive_comments.fileid
	      AND comment <> \"\"
	      ORDER  BY offensive_comments.timestamp DESC 
	      LIMIT 100";
	$result = tmbo_query( $sql );
	
	$seen = array();
	$count = 0;
	while( ($row = mysqli_fetch_assoc( $result )) && $count < 15) {
		$upload = new Upload($row);
		
		if(array_key_exists($upload->id(), $seen)) continue;
		$seen[$upload->id()] = true;
		$count++;

		// XXX: rejigger the query and use Link::comment
		?><li><div><a href="<?= Link::thread($upload) ?>#<?= $row['commentid'] ?>"><?= $upload->htmlFilename() ?></a></div></li><?php

	}
?>
	</ul>
<?php

}


function handleSetStatusSubmission() {
	// an authenticated request is not the same as a request we asked for.
	if(!csrfValid()) {
		trigger_error("rejected an account change with a missing or wrong csrf token", E_USER_ERROR);
		exit;
	}

	$usrid =  sqlEscape( $_POST['userid'] );
	$status =  sqlEscape( $_POST['set_account_status'] );
	$email =  sqlEscape( $_POST['set_email'] );

	$them = new User($usrid);
	$content = "previously ".$them->username()."'s status was ".$them->status();

	$changed = $them->status() != $status;

	if( is_intger($usrid ) && $status !== "" ) {
		$isEmailValid = preg_match( '/^[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+$/', $email ) > 0;
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
	$row = mysqli_fetch_row($result);
	$options = explode("','",preg_replace("/(enum|set)\('(.+?)'\)/","\\2",$row[1]));
	
	?>
		<div style="margin:8px 0px;">
			<form action="<?= $_SERVER['PHP_SELF']?>" method="post">
				<input type="hidden" name="c" value="user"/>
				<input type="hidden" name="csrf" value="<?= htmlEscape(csrfToken()) ?>"/>
				<input type="hidden" name="userid" value="<?= $id ?>"/>
				<select name="set_account_status">
				<?php
				foreach( $options as $option ) {
					$selected = ($option == $status ) ? "selected=\"selected\"" : "";
					?>		
						<option value="<?=$option?>" <?= $selected ?>><?=$option?></option>
					<?php
				}
				?>
				</select>
				<input type="text" name="set_email" value="<?= htmlEscape($email) ?>"/>
				<input type="submit" value="update account"/>
			</form>
		</div>
	<?php

}

function votingRecord( $id, $name ) {
	if(me()->squelched($id)) {
		return;
	}
	
	$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id";
	$result = tmbo_query( $sql );
	$comments = mysqli_fetch_row($result);
	$comments = $comments[0];

	if($comments > 1000) {
		?><h3><?php echo $name ?> has made <?= $comments ?>  votes/comments<?php
	} else {
		$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id AND vote = 'this is good'";
		$result = tmbo_query( $sql );
		$good = mysqli_fetch_row($result);
		$good = $good[0];
		
		$sql = "SELECT COUNT(*) FROM offensive_comments WHERE userid = $id AND vote = 'this is bad'";
		$result = tmbo_query( $sql );
		$bad = mysqli_fetch_row($result);
		$bad = $bad[0];
		
		?><h3><?php echo $name ?>'s voting record: +<?= $good ?> -<?= $bad ?><?php
	}

?> (<a href="<?= Link::content("votedetail")."&userid=".$id ?>">click for details</a>)</h3>
<?php

}

function contributions($usrid) {
	global $them;
	if(me()->squelched($them)) {
		return;
	}
	
	?><h3>
		<a href="<?= Link::mainpage() ?>&userid=<?= $usrid?>"><?= $them->username() ?>'<?= substr($them->username(), -1) == "s" ? "" : "s" ?> contributions to society:</a>
		<!-- +<?php echo isset($good_votes) ? $good_votes : 0; ?> -<?php echo isset($bad_votes) ? $bad_votes : 0; ?> (<a href="./?c=uservotedetail&userid=<?= $usrid ?>">click for details</a>)--></h3>
	<ul class="imagelist">
<?php
	$uploads = core_getuploads(array("userid" => $usrid));
	foreach($uploads as $upload) {
		?>
		<li>
			<div><a href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename(true) ?></a></div>
			<div><a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?=$upload->commentLabel(!($upload->type() == 'topic'));?></a></div>
		</li>
		<?php
	}
?>
	</ul>
<?php
}

?>
