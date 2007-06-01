<?php 
	session_start();
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();
	
	$sql = "select userid from users where username like '" . $_REQUEST['finduser'] . "'";

	$result = mysql_query($sql);

	echo mysql_error();

	$row = mysql_fetch_array( $result );

	if( mysql_num_rows( $result ) == 1 ) {
		header( "Location:./?c=user&userid=" .  $row['userid'] );
	}
	else {
		header( "Location: " . $_SERVER['HTTP_REFERER'] );
	}

?>

<html>
<body>

not found.<br/>
<?
	include( 'finduserform.php' );
?>
</body>
</html>
