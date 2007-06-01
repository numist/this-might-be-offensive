<?
	session_start();
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	$uid = $_SESSION['userid'];

	if( is_numeric( $uid ) ) {
		
		$link = openDbConnection();
		$sql = "DELETE FROM offensive_bookmarks WHERE userid=$uid";
		mysql_query( $sql );
	}

	header( "Location: ./" );

?>
