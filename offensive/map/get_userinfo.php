<?php
// this code grabs all the different pieces of user info that we'd like to
// show on a google infowindow.

set_include_path("../..");
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
require_once("offensive/assets/classes.inc");

// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

$user = (isset($_GET['user']) && is_intger($_GET['user'])) ? $_GET['user'] : "";
if($user == "") trigger_error("no user argument", E_USER_ERROR);

$user = new User($user);
$avatar = $user->yearbook();

if($avatar == false) {
	// show a default image if there is no thumbnail
	$thumb = "<img src='/tmbologo.gif' width='50' height='33' />";
} else {
	$info = getimagesize($avatar->thumb());
	
	// XXX: this could do with some filtering.
	$thumb = "<a href='/offensive/pages/pic.php?id=".$avatar->id()."' target='_blank'><img src='" . $avatar->thumbURL() . "' ".$info[3]." border='0' /></a>";
}

$referer = $user->referred_by();
$refer = "";

if($referer) {
	if($referer->location()) {
		$refer = $user->username()." was invited by <a href='javascript:posse_click(".$referer->id().");'>"
		         .$referer->username()."</a>";
	} else {
		$refer = $user->username()." was invited by ".$referer->username();
	}
}

$possearr = $user->posse();
if(count($possearr) == 0) {
	$posse = "";
	$overflow = "";
} else {
	$posse = "<p style='line-height: 10px; margin: 4px 0px 3px 0px;'><a style='text-decoration: none;' href='/offensive/?c=posse&amp;userid=".$user->id()."'>".$user->username()." has a posse</a>";
	
	$posse_markers = 0;
	$posse_list = "";
	foreach($possearr as $posser) {
		if($posser->location()) {
			$posse_markers++;
			$posse_list .= "<li><a href='javascript:posse_click(".$posser->id().");'>".$posser->username()."</a></li>";
		}
	}
	
	if(count($possearr) != $posse_markers && $posse_markers != 0)
		$posse .= " (".count($possearr).", $posse_markers shown)";
		
	$posse .= "</p>";
	
	$overflow = ($posse_markers > 15) ? "height: 130px; overflow: auto;" : "";
}

// we have all the info, render.
?><div>
	<div style='<?= $posse_markers == 0 ?'': "border-bottom: 1px solid black; " ?>height: auto;'>
		<table>
			<tr>
				<td><?= $thumb ?></td>
				<td><h2><a style="text-decoration: none;" href="/offensive/?c=user&userid=<?= $user->id() ?>">&nbsp;&nbsp;<?= $user->username() ?></a></h2></td>
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
