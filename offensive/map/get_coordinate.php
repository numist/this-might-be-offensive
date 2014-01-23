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

$user = me()->id();
if($user == "") trigger_error("no user argument", E_USER_ERROR);

$locations = core_getlocation(array("userid" => $user));


header("Content-type: application/json");
$data = array(
	'markers'  => array()
);
foreach ($locations as $location){
	$data['markers'][] = array(
		'userid' => $location['userid'],
		'lat'    => $location['latitude'],
		'lon'    => $location['longitude']
	);
}
echo json_encode($data);

?>
