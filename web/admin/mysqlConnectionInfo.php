<?php

	$db_url = "66.228.121.115";
	$db_user = "thismig_tmbo";
	$db_pw = "1A2B3C!D";

	$link = null;

	function openDbConnection() {

		global $db_url, $db_user, $db_pw, $link;

		if( $link == null ) {
			$link = @mysql_pconnect( $db_url, $db_user, $db_pw )
				or header( "Location: http://thismight.be/offensive/index.outoforder.php" );
//				or die( "<br><br><br>Unable to connect to database." );
			
			mysql_select_db("thismig_themaxx")
				or die("<br><br>Could not select database" );

		}

		return $link;		
	}
	
?>
