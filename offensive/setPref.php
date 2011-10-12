<?php
	set_include_path("..");
	require_once('offensive/assets/header.inc');

	mustLogIn();

	require_once( 'admin/mysqlConnectionInfo.inc' ); 
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/functions.inc');
	require_once("offensive/assets/classes.inc");
	
	$me = me();

	$prefname = sqlEscape( array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : "");
	$value = sqlEscape( array_key_exists("v", $_REQUEST) ?$_REQUEST['v'] : "");

	if(strlen($prefname) > 0) {
		$me->setPref($prefname, $value);
	}

	if( array_key_exists("sq", $_REQUEST) && is_intger( $_REQUEST['sq'] ) ) {
		$me->squelch($_REQUEST['sq']);
	}

	if( array_key_exists("unsq", $_REQUEST) && is_intger( $_REQUEST['unsq'] ) ) {
		$me->unsquelch($_REQUEST['unsq']);
	}

	if(array_key_exists("HTTP_REFERER", $_SERVER)) {
		header( "Location: " . $_SERVER['HTTP_REFERER'] );
	} else { ?>
		<html><head><script type="text/javascript">history.go(-1);</script></head><body /></html>
	<? } ?>