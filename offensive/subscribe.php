<?
	set_include_path("..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/assets/comments.inc");
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	require_once("offensive/assets/classes.inc");
	if(!isset($link) || !$link) $link = openDbConnection();

	mustLogIn();

	$fileid = array_key_exists("fileid", $_REQUEST) ? $_REQUEST['fileid'] : "";
	
	$upload = new Upload($fileid);
	
	if( array_key_exists("un", $_REQUEST) && $_REQUEST['un'] == 1 ) {
		$upload->unsubscribe();
	}
	else {
		$upload->subscribe();
	}

	if(array_key_exists("HTTP_REFERER", $_SERVER)) {
		header( "Location: " . $_SERVER['HTTP_REFERER']);
	} else {
		echo "<html><head><script type=\"text/javascript\">history.go(-1);</script></head><body /></html>";
	}
	exit;

?>