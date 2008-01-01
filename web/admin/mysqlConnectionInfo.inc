<?php

// PHP5 has a super little function called parse_ini_file() which will do most of the work here, so
// we'll grab the contents of the settings file, located in this directory and .config into an array
if(!($config = parse_ini_file("admin/.config"))) {
	header( "Location: http://thismight.be/offensive/index.outoforder.php?noconfig" );
}

// Set $link null
$link = null;

// Generic database connection function
function openDbConnection() {
		// Globalise $config and $link
		global $config, $link;

		// If there isn't $link currently, then open up the connection, if it fails then forward the user
		// to the out of order URL.
		if( !$link ) {
			if(!($link = mysql_pconnect($config["database_host"],$config["database_user"],$config["database_pass"]))) {
				file_put_contents("/tmp/tmbo.log", mysql_error()."\n", FILE_APPEND);
				header( "Location: http://thismight.be/offensive/index.outoforder.php?noconnect");
			}
			
			// We got a connection, so now select our database, or again forward to the out of order URL.
			if(!mysql_select_db($config["database_name"],$link)) {
				file_put_contents("/tmp/tmbo.log", mysql_error()."\n", FILE_APPEND);
				header( "Location: http://thismight.be/offensive/index.outoforder.php?selectdb" );
			}
		}
		return $link;		
}
?>
