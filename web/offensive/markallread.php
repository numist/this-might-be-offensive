<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc');
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	mustLogIn();

	$uid = $_SESSION['userid'];
		
	$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$uid";
	tmbo_query( $sql );

	header( "Location: ./" );

?>