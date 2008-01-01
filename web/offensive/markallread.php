<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc');

	include_once( 'admin/mysqlConnectionInfo.inc' );

	$uid = $_SESSION['userid'];

	if( is_numeric( $uid ) ) {
		
		if(!isset($link) || !$link) $link = openDbConnection();
		$sql = "DELETE FROM offensive_bookmarks WHERE userid=$uid";
		mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	}

	header( "Location: ./" );

?>
