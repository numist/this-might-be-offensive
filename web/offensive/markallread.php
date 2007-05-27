<?
	session_start();
	require_once( '../admin/mysqlConnectionInfo.php' );

	$uid = $_SESSION['userid'];

	if( is_numeric( $uid ) ) {
		
		$link = openDbConnection();
		$sql = "DELETE FROM offensive_bookmarks WHERE userid=$uid";
		mysql_query( $sql );
	}

	header( "Location: ./" );

?>
