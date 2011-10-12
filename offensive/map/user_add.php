<?php
// this should move to some api call.
set_include_path("../..");

require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
        
// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

// users can only change their own location.
$user = $_SESSION['userid'];

$lat = (isset($_GET['lat']) && is_numeric($_GET['lat'])) ? $_GET['lat'] : "";
if($lat == "") trigger_error("no latitude set", E_USER_ERROR);

$long = (isset($_GET['long']) && is_numeric($_GET['long'])) ? $_GET['long'] : "";
if($long == "") trigger_error("no longitude set", E_USER_ERROR);

$sql = "REPLACE INTO maxxer_locations (userid, x, y, mapversion) VALUES( $user, $lat, $long, 'google' )";
$result = tmbo_query( $sql );

?>
