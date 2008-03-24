<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc');

	mustLogIn();

	include_once( 'admin/mysqlConnectionInfo.inc' );

	$uid = $_SESSION['userid'];
		
	if(!isset($link) || !$link) $link = openDbConnection();
	$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$uid";
	tmbo_query( $sql );

	header( "Location: ./" );

?>