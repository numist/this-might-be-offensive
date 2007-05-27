<?php

	$db_url = "mysql.rocketsheep.com";
	$db_user = "db_themaxx";
	$db_pw = "db_password_goes_here";

	$link = null;

	function openDbConnection() {

		global $db_url, $db_user, $db_pw, $link;

		if( $link == null ) {
			$link = @mysql_pconnect( $db_url, $db_user, $db_pw )
				or header( "Location: http://tmbo.org/offensive/index.outoforder.php" );
//				or die( "<br><br><br>Unable to connect to database." );
			
			mysql_select_db("themaxx")
				or die( "<br><br>Could not select database" );

		}

		return $link;		
	}
	
?>
