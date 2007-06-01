<?
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	$sql = "SELECT id, filename
			FROM offensive_uploads
			ORDER BY id DESC
			LIMIT 1";
	$result = mysql_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$filename = $row['filename'];
	
	if( file_exists( "images/picpile/$filename" ) ) {
		header( "Location: images/picpile/$filename" );
	}

?>