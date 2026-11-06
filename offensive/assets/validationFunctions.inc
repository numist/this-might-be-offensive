<?php
	require_once( "offensive/classes/statusMessage.inc" );
	require_once( "offensive/assets/functions.inc" );

	// structure only.  the character set is narrow enough that a passing address
	// is safe as a mail() recipient: no spaces, commas or line breaks.
	function isValidEmailFormat( $email ) {
		return preg_match( '/^[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+$/D', (string)$email ) > 0;
	}

	function isValidEmail( $email ) {
	
		$valid = isValidEmailFormat( $email ) && strpos( $email, "mailinator.com" ) == false;
	
		if( ! $valid ) {
			return new statusMessage( false, "The email address provided is invalid, either because it contains structural errors or is from an unsupported provider (mailinator, etc)." );
		}
	
		// Include, and check we've got a connection to the database.
		require_once( 'admin/mysqlConnectionInfo.inc' );
		if(!isset($link) || !$link) $link = openDbConnection();

		$sql = "SELECT count(*) AS theCount FROM users WHERE email = '" . sqlEscape( $email ) . "'";
		$result = tmbo_query( $sql );
		$row = mysqli_fetch_assoc( $result );
		if( $row['theCount'] > 0 ) {
			return new statusMessage( false, "An account with that email address already exists." );
		}
		return new statusMessage( true, "" );
	}
	
	function isValidUsername( $uName ) {
		// This function must not allow any strings that can be used for SQL injection!
		return preg_match( "/^[a-z0-9_\-\.]+$/i", $uName );
	}
	
	function isValidPassword( $pw ) {
		return strlen($pw) >= 8;
	}

?>
