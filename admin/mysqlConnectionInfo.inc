<?php

// PHP5 has a super little function called parse_ini_file() which will do most of the work here, so
// we'll grab the contents of the settings file, located in this directory and .config into an array
if(!($config = parse_ini_file("admin/.config"))) {
	trigger_error("No configuration file found", E_USER_ERROR);
	exit;
}

require_once("offensive/assets/Predis/Autoloader.php");
Predis\Autoloader::register();

// initialize database links
$link = null;
$redis = null;

// Generic database connection function
function openDbConnection() {
		global $config, $link, $redis;

		// If there isn't $link currently, then open up the connection, if it fails then forward the user
		// to the out of order URL.
		if( !$link ) {
			if(!($link = mysql_pconnect($config["database_host"],$config["database_user"],$config["database_pass"]))) {
				trigger_error(mysql_error(), E_USER_ERROR);
			}
			
			// We got a connection, so now select our database, or again forward to the out of order URL.
			if(!mysql_select_db($config["database_name"],$link)) {
				trigger_error(mysql_error(), E_USER_ERROR);
			}
		}
		
		// Overloading this into the db connect since the combination is our "database"
		if(!$redis) {
			$redis = new Predis\Client();
		}
		return $link;		
}
?>
