<?php

// this code creates an XML file for one editting one specific marker

set_include_path("../..");

// set up the normal TMBO environment
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
require_once("offensive/assets/classes.inc");
require_once("offensive/assets/core.inc");
        
// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

$user = (isset($_GET['user']) && is_intger($_GET['user'])) ? $_GET['user'] : "";
if($user == "") trigger_error("no user argument", E_USER_ERROR);

$locations = core_getlocation(array("userid" => $user));

if(count($locations) == 0) {
	exit;
}

header("Content-type: text/xml");
echo '<markers>';
foreach ($locations as $location){
	// ADD TO XML DOCUMENT NODE
	echo '<marker ';
	echo 'userid="' . $location['userid'] . '" ';
	echo 'lat="' . $location['latitude'] . '" ';
	echo 'lon="' . $location['longitude'] . '" ';
	echo '/>';
}
echo '</markers>';

?>
