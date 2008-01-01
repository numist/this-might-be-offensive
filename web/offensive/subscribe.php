<?
	set_include_path("..");
	require_once("offensive/assets/header.inc");

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	$uid = $_SESSION['userid'];
	$fileid = $_REQUEST['fileid'];
	
	if( ! is_numeric( $uid ) || ! is_numeric( $fileid ) ) {
		header( "Location: ./" );
	}

	if( $_REQUEST['un'] == 1 ) {
		unsubscribe( $uid, $fileid );
	}
	else {
		subscribe( $uid, $fileid );
	}

	header( "Location: " . $_SERVER['HTTP_REFERER'] );
	exit;

	function subscribe( $uid, $fid ) {
		$link = openDbConnection();
		$sql = "insert into offensive_subscriptions (userid, fileid) values ( $uid, $fid )";
		mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	}

	function unsubscribe( $uid, $fid ) {
		$link = openDbConnection();
		$sql = "delete from offensive_subscriptions where userid=$uid and fileid=$fid";
		mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	}

?>
