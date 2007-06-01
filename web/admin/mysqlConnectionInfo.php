<?php
// PHP5 has a super little function called parse_ini_file() which will do most of the work here, so
// we'll grab the contents of the settings file, located in this directory and .config into an array
$config = parse_ini_file(".config");

// Set $link null
$link = null;

// Generic database connection function
function openDbConnection() {
		// Globalise $config and $link
		global $config, $link;

		// If there isn't $link currently, then open up the connection, if it fails then forward the user
		// to the out of order URL.
		if( $link == null ) {
			$link = mysql_pconnect($config["database_host"],$config["database_user"],$config["database_pass"])
				or header( "Location: http://thismight.be/offensive/index.outoforder.php");
			
			// We got a connection, so now select our database, or again forward to the out of order URL.
			mysql_select_db($config["database_name"],$link)
				or header( "Location: http://thismight.be/offensive/index.outoforder.php");
		}
		return $link;		
}
?>
