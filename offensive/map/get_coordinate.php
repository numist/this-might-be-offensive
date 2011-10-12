<?php

// this code creates an XML file for one editting one specific marker

set_include_path("../..");

// set up the normal TMBO environment
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
        
// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

$user = (isset($_GET['user']) && is_intger($_GET['user'])) ? $_GET['user'] : "";
if($user == "") trigger_error("no user argument", E_USER_ERROR);

$sql = "SELECT userid,x,y FROM maxxer_locations WHERE mapversion='google' AND userid='$user'";

$result = mysql_query($sql);
if(!$result) {
	exit;
}
header("Content-type: text/xml");
echo '<markers>';
while ($row = @mysql_fetch_assoc($result)){
	// ADD TO XML DOCUMENT NODE
	echo '<marker ';
	echo 'userid="' . $row['userid'] . '" ';
	echo 'lat="' . $row['x'] . '" ';
	echo 'lon="' . $row['y'] . '" ';
	echo '/>';
}
echo '</markers>';

?>
