<?
	if( $_SESSION['userid'] <> 151 ) {
		header( "Location: ./" );
	}

	require_once( '../admin/mysqlConnectionInfo.php' );

	$link = openDbConnection();

	$sql = "INSERT INTO offensive_comments ( userid, fileid, vote )
				VALUES( 151, 44604, 'this is bad' )
			";

	for( $i = 0; $i < 200; $i++ ) {
		mysql_query( $sql );
	}

?>