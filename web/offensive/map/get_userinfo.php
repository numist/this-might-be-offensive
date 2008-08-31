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
	$thumb = "<img src='/tmbologo.gif' width='50' />";
} else {
	$ret = mysql_fetch_assoc($result);
	extract($ret);
	$thumb = "<IMG SRC='" . getThumbURL($id,$filename,$timestamp,$type) . "' />";
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
if($referer != "" && $rfb != $user) {
	$refer = "$username was invited by <a href='javascript:posse_click($rfb);'>$referer</a>";
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



// get the posse info
$sql = "SELECT userid,username as posse_user FROM users WHERE userid != '$user' AND referred_by = '$user' ORDER BY username";
$posse_list = "";
$result = tmbo_query($sql);
$num_rows = mysql_num_rows($result);
if($num_rows == 0) {
	$posse = "";
	$overflow = "";
} else {
	$posse = "<p style='font-size: 11px; line-height: 10px; margin: 4px 0px 3px 0px;'><b><a style='text-decoration: none;' href='/offensive/?c=posse&amp;userid=$user'>$username has a posse</a></p>";
	while ($ret = @mysql_fetch_assoc($result)) {
		extract($ret);
		if(isset($markers["$userid"])) {
			$posse_list .= "<a style='text-decoration: none;' href='javascript:posse_click($userid);'>$posse_user</a><br />";
		} else {
			$posse_list .= "<a style='text-decoration: none; color: black;'><i>$posse_user</i><a><br />";
		}
	}
	$overflow = ($num_rows > 15) ? "height: 130px; overflow: auto;" : "";
}



// we have all the info, render.
?><div>
	<div style='<?= $posse == "" ?'': "border-bottom: 1px solid black; " ?>height: auto;'>
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
	<div style="margin: 5px 0 0 20px; font-size: 10px; line-height: 18px; text-decoration: none; <?= $overflow ?>">
		<?= $posse_list ?>
		<? } ?>
	</div>
</div>