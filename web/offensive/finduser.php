<?php
	set_include_path("..");
	session_start();
	require_once( 'offensive/assets/header.inc' );

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/functions.inc');
	
	$sql = "select userid from users where username like '" . sqlEscape($_REQUEST['finduser']) . "'";

	$result = mysql_query($sql) or trigger_error(mysql_error(), E_USER_ERROR);

	echo mysql_error();

	$row = mysql_fetch_array( $result );

	if( mysql_num_rows( $result ) == 1 ) {
		header( "Location:./?c=user&userid=" .  $row['userid'] );
	}
	else {
		header( "Location: " . $_SERVER['HTTP_REFERER'] );
	}

?>
