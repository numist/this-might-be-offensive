<?php
	require_once( "offensive/assets/functions.inc" );
	require_once( "offensive/assets/validationFunctions.inc" );
	require_once( "offensive/assets/logn.inc" );
	require_once( "offensive/classes/statusMessage.inc" );

	/* change the logged-in user's password from the settings page.
	 *
	 * the current password is required even though the session is authenticated:
	 * a stolen session or remember cookie should not be enough to lock the owner
	 * out.  wrong guesses count toward the login form's per-IP limit, so this
	 * cannot be used to brute-force a password the login form would stop.
	 *
	 * every remember cookie is keyed with the password hash, so changing it logs
	 * out every other browser that was remembered.  the browser making the change
	 * gets a fresh cookie if it had one, so it doesn't log itself out too.
	 *
	 * returns a statusMessage.
	 */
	function passwordChange( $old, $new, $confirm )
	{
		if( TMBO::readonly() ) {
			return new statusMessage( false, "This session is read-only." );
		}

		if( !canLogIn() ) {
			return new statusMessage( false, "Too many failed attempts. Try again later." );
		}

		$old = (string)$old;
		$new = (string)$new;
		$uid = (int)me()->id();

		$result = tmbo_query( "SELECT password FROM users WHERE userid = $uid LIMIT 1" );
		$row = mysqli_num_rows( $result ) == 1 ? mysqli_fetch_assoc( $result ) : false;
		if( !$row || !tmbo_hash_equals( (string)$row['password'], sha1( $old ) ) ) {
			failedLogIn( me()->username() );
			return new statusMessage( false, "Wrong password." );
		}

		if( $new !== (string)$confirm ) {
			return new statusMessage( false, "The new passwords don't match." );
		}
		// same rule as registration and the password reset.
		if( !isValidPassword( $new ) ) {
			return new statusMessage( false, "Passwords must be at least 8 characters long." );
		}

		$hash = sha1( $new );
		tmbo_query( "UPDATE users SET timestamp = timestamp, password = '$hash' WHERE userid = $uid LIMIT 1" );

		if( array_key_exists( REMEMBER_COOKIE, $_COOKIE ) ) {
			issueRememberCookie( me() );
		}

		return new statusMessage( true, "Your password has been changed." );
	}
?>
