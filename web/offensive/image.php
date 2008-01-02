<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc');

	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	$sql = "SELECT id, filename
			FROM offensive_uploads
			ORDER BY id DESC
			LIMIT 1";
	$result = tmbo_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$filename = $row['filename'];
	
	if( file_exists( "images/picpile/$filename" ) ) {
		header( "Location: images/picpile/$filename" );
	}

?>
