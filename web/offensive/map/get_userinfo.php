<?php
// this code grabs all the different pieces of user info that we'd like to
// show on a google infowindow.

set_include_path("../..");
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");

// authentication
mustLogIn("http");

$user = (isset($_GET['user']) && is_numeric($_GET['user'])) ? $_GET['user'] : "";
if($user == "") trigger_error("no user argument", E_USER_ERROR);



// get the thumbinfo
$sql = "SELECT up.id, up.userid, up.filename, up.timestamp, up.type FROM offensive_uploads up, users u WHERE u.userid = up.userid AND up.userid = '$user' AND up.status = 'normal' AND up.type = 'avatar' ORDER BY up.id DESC limit 1";

$result = tmbo_query($sql);
if(mysql_num_rows($result) == 0) {
	// show a default image if there is no thumbnail
	$thumb = "<img src='/tmbologo.gif' width='50' height='33' />";
} else {
	$ret = mysql_fetch_assoc($result);
	extract($ret);
	
	$info = getimagesize(getThumb($id, $filename, $timestamp, $type));
	
	$thumb = "<a href='/offensive/pages/pic.php?id=$id' target='_blank'><img src='" . getThumbURL($id,$filename,$timestamp,$type) . "' ".$info[3]." border='0' /></a>";
}



// get the username, referred_by, and username of referred_by
$sql = "SELECT username,referred_by as rfb, (SELECT username from users where userid=rfb) as referer FROM users WHERE userid='$user'";
$result = tmbo_query($sql);
if(mysql_num_rows($result) == 0) {
	$username = '';
	$referer = "";
} else {
	$ret = mysql_fetch_assoc($result);
	extract($ret);
}

// to be able to find which posse members are clickable, we need info
// on the map markers. This could probably be done better, but for now this
// SQL call is pretty cheap.
$sql = "SELECT userid FROM maxxer_locations WHERE mapversion='google';";
$result = tmbo_query($sql);
$markers = array();
if($result) {
	while ($ret = @mysql_fetch_assoc($result)) {
		extract($ret);
		$markers["$userid"] = 1;
	}
}

if($referer != "" && $rfb != $user) {
	if(isset($markers["$rfb"])) {
		$refer = "$username was invited by <a href='javascript:posse_click($rfb);'>$referer</a>";
	} else {
		$refer = "$username was invited by $referer";
	}
}

// get the posse info
$sql = "SELECT userid,username as posse_user FROM users WHERE userid != '$user' AND referred_by = '$user' AND account_status != 'locked' ORDER BY username";
$posse_list = "";
$result = tmbo_query($sql);
$num_rows = mysql_num_rows($result);
if($num_rows == 0) {
	$posse = "";
	$overflow = "";
} else {
	$posse = "<p style='line-height: 10px; margin: 4px 0px 3px 0px;'><a style='text-decoration: none;' href='/offensive/?c=posse&amp;userid=$user'>$username has a posse</a>";
	
	$posse_markers = 0;
	while ($ret = @mysql_fetch_assoc($result)) {
		extract($ret);
		if(isset($markers["$userid"])) {
			$posse_markers++;
			$posse_list .= "<li><a href='javascript:posse_click($userid);'>$posse_user</a></li>";
		}
	}
	
	if($num_rows != $posse_markers && $posse_markers != 0) $posse .= " ($num_rows, $posse_markers shown)";
	$posse .= "</p>";
	
	$overflow = ($posse_markers > 15) ? "height: 130px; overflow: auto;" : "";
}



// we have all the info, render.
?><div>
	<div style='<?= $posse_markers == 0 ?'': "border-bottom: 1px solid black; " ?>height: auto;'>
		<table>
			<tr>
				<td><?= $thumb ?></td>
				<td><h2><a style="text-decoration: none;" href="/offensive/?c=user&userid=<?= $user ?>">&nbsp;&nbsp;<?= $username ?></a></h2></td>
			</tr>
		</table>
		<?= $refer ?>
		<? if($posse != "") {
			echo $posse; ?>
	</div>
	<div style="margin: 5px 0 0 0; font-size: 10px; line-height: 18px; text-decoration: none; <?= $overflow ?>">
		<ul>
		<?= $posse_list ?>
		</ul>
		<? } ?>
	</div>
</div>
