<?
	require_once( "classes/statusMessage.php" );

	function isValidEmail( $email ) {
	
		$valid = preg_match( '/[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+/', $email ) > 0 && strpos( $email, "mailinator.com" ) == false;
	
		if( ! $valid ) {
			return new statusMessage( false, "The email address provided is invalid, either because it contains structural errors or is from an unsupported provider (mailinator, etc)." );
		}
	
		// Include, and check we've got a connection to the database.
		include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

		$sql = "SELECT count(*) AS theCount FROM users WHERE email = '" . $email . "'";
		$row = mysql_fetch_assoc( mysql_query( $sql ) );
		if( $row['theCount'] > 0 ) {
			return new statusMessage( false, "An account with that email address already exists." );
		}
		return new statusMessage( true, "" );
	}
	
	function isValidUsername( $uName ) {
		return preg_match( "/^[a-z0-9_\-\.]+$/i", $uName );
	}
	
	function isValidPassword( $pw ) {
		return preg_match( "/[a-z0-9_\-\.]{5}/i", $pw );
	}

?>