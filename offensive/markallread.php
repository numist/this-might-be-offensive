<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc');
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	mustLogIn();

	$uid = me()->id();
		
	$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$uid";
	tmbo_query( $sql );

	if(array_key_exists("HTTP_REFERER", $_SERVER)) {
		header( "Location: " . $_SERVER['HTTP_REFERER'] );
	} else { ?>
		<html><head><script type="text/javascript">history.go(-1);</script></head><body /></html>
	<? } ?>