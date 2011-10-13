<?php
// this should move to some api call

set_include_path("../..");

error_log("sql\n",3,"/tmp/log");
// set up the normal TMBO environment
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
        
// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

// users can only remove their own location.
$user = me()->id();

$sql = "DELETE FROM maxxer_locations WHERE userid='$user'";
$result = tmbo_query( $sql );
?>