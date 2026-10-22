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
			// Callers check for false returns (`... or tmbo_error_handler()`), so keep
			// mysqli from throwing like it does by default since PHP 8.1.
			mysqli_report(MYSQLI_REPORT_OFF);

			// "p:" makes it persistent, like mysql_pconnect() was.
			if(!($link = mysqli_connect("p:".$config["database_host"],$config["database_user"],$config["database_pass"]))) {
				trigger_error(mysqli_connect_error(), E_USER_ERROR);
			}

			// The tables are latin1 but hold UTF-8 bytes; talking latin1 passes them
			// through untouched, as the old mysql extension did.
			if(!mysqli_set_charset($link, "latin1")) {
				trigger_error(mysqli_error($link), E_USER_ERROR);
			}

			// The old MySQL wasn't strict, and the code relies on it: IPv6 addresses,
			// for one, don't fit the IP columns and used to be silently truncated.
			if(!mysqli_query($link, "SET SESSION sql_mode = 'NO_ENGINE_SUBSTITUTION'")) {
				trigger_error(mysqli_error($link), E_USER_ERROR);
			}

			// We got a connection, so now select our database, or again forward to the out of order URL.
			if(!mysqli_select_db($link, $config["database_name"])) {
				trigger_error(mysqli_error($link), E_USER_ERROR);
			}
		}
		
		// Overloading this into the db connect since the combination is our "database"
		if(!$redis) {
			$redis_host = isset($config['redis_host']) ? $config['redis_host'] : 'localhost';
			$redis = new Predis\Client(['host' => $redis_host]);
		}
		return $link;		
}
?>
