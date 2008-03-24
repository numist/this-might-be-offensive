<?
	set_include_path("..");
	require_once("offensive/assets/header.inc");

	mustLogIn();

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	$uid = array_key_exists("userid", $_SESSION) ? $_SESSION['userid'] : "";
	$fileid = array_key_exists("fileid", $_REQUEST) ? $_REQUEST['fileid'] : "";
	
	if( array_key_exists("un", $_REQUEST) && $_REQUEST['un'] == 1 ) {
		unsubscribe( $uid, $fileid );
	}
	else {
		subscribe( $uid, $fileid );
	}

	if(array_key_exists("HTTP_REFERER", $_SERVER)) {
		header( "Location: " . $_SERVER['HTTP_REFERER']);
	} else {
		echo "<html><head><script type=\"text/javascript\">history.go(-1);</script></head><body /></html>";
	}
	exit;

	function subscribe( $uid, $fid ) {
		$sql = "SELECT * FROM offensive_subscriptions WHERE userid = $uid AND fileid = $fid";
		if(mysql_num_rows(tmbo_query($sql)) > 0) return;

		$link = openDbConnection();
		$sql = "insert into offensive_subscriptions (userid, fileid) values ( $uid, $fid )";
		tmbo_query( $sql );
	}

	function unsubscribe( $uid, $fid ) {
		$link = openDbConnection();
		$sql = "delete from offensive_subscriptions where userid=$uid and fileid=$fid";
		tmbo_query( $sql );
	}

?>