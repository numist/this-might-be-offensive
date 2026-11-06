<?php
	require_once( "offensive/assets/functions.inc" );
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/validationFunctions.inc" );
	require_once( "offensive/assets/logn.inc" );
	require_once( "offensive/classes/statusMessage.inc" );

	/* self-service email change.
	 *
	 * 1. the user asks for a new address on the settings page, giving their
	 *    current password.  the session alone is not enough: anyone holding a
	 *    stolen session or remember cookie could otherwise point the account at
	 *    their own mailbox and then take it over through the password reset.
	 * 2. a link goes to the NEW address.  nothing changes until it is followed,
	 *    so a typo cannot hand the account's reset emails to a stranger.
	 * 3. following it while logged in as the same user makes the change, and the
	 *    OLD address is told about it.  the stranger case again: whoever owns a
	 *    mistyped address can click the link, but not as this user.
	 *
	 * the code is stateless, like the password reset's.  it signs the user's
	 * current email, the new email and the password hash, so it dies once the
	 * change is made, if the password changes, or after a day or two.  no table,
	 * no migration.
	 *
	 * it is keyed with pwreset_salt rather than a key of its own, so deploying
	 * this needs no admin/.config change.  the "email-change" prefix keeps its
	 * input from ever matching a reset code's, which starts with a username and
	 * usernames cannot contain ":".  rotating pwreset_salt kills both kinds of
	 * outstanding link, which is what you would want anyway.
	 */

	define( "EMAIL_CHANGE_COOLDOWN", 600 );

	function emailChangeCode( $userRow, $newEmail, $day )
	{
		$input = "email-change:" . (string)$userRow['email'] . "\n" . $newEmail . "\n" .
		         $userRow['password'] . "\n" . $day . tmbo_secret( "pwreset_salt" );
		return tmbohash( $userRow['userid'], $input );
	}

	function emailChangeUserRow( $userid )
	{
		$userid = (int)$userid;
		$result = tmbo_query( "SELECT userid, username, email, password FROM users WHERE userid = $userid" );
		return mysqli_num_rows( $result ) == 1 ? mysqli_fetch_assoc( $result ) : false;
	}

	/* is $newEmail acceptable as $userRow's address?  returns a statusMessage. */
	function emailChangeValidate( $userRow, $newEmail )
	{
		if( strlen( $newEmail ) > 60 ) {
			return new statusMessage( false, "That address is too long." );
		}
		if( strcasecmp( $newEmail, (string)$userRow['email'] ) == 0 ) {
			return new statusMessage( false, "That's already your address." );
		}
		// format, blocked providers, and one account per address, as registration does.
		return isValidEmail( $newEmail );
	}

	/* handle the settings page form.  returns a statusMessage. */
	function emailChangeRequest( $newEmail, $password )
	{
		if( TMBO::readonly() ) {
			return new statusMessage( false, "This session is read-only." );
		}

		// wrong passwords here count toward the same per-IP limit as the login form,
		// so this cannot be used to guess a password the login form would stop.
		if( !canLogIn() ) {
			return new statusMessage( false, "Too many failed attempts. Try again later." );
		}

		$row = emailChangeUserRow( me()->id() );
		if( !$row || !tmbo_hash_equals( $row['password'], sha1( (string)$password ) ) ) {
			failedLogIn( me()->username() );
			return new statusMessage( false, "Wrong password." );
		}

		$newEmail = trim( (string)$newEmail );
		$valid = emailChangeValidate( $row, $newEmail );
		if( !$valid->status ) {
			return $valid;
		}

		// every send is mail to an address the user typed, so cap how often a
		// session can do it.
		if( array_key_exists( "email_change_sent", $_SESSION ) &&
		    $_SESSION['email_change_sent'] > time() - EMAIL_CHANGE_COOLDOWN ) {
			return new statusMessage( false, "A confirmation email was sent a few minutes ago. Check for it, or try again later." );
		}

		// the format check allows only [A-Za-z0-9-_.@], so the address is safe in a
		// mail() recipient and in a URL without further escaping; urlencode anyway.
		$code = emailChangeCode( $row, $newEmail, gmdate( "Y-m-d" ) );
		$link = "https://thismight.be" . Link::content( "settings" ) .
		        "&emailcode=" . urlencode( $code ) . "&newemail=" . urlencode( $newEmail );

		$message = "Someone (hopefully you) asked to use this address for the [this might be offensive] account \"" . $row['username'] . "\".

To confirm, visit this link while logged in as " . $row['username'] . ":

$link

The link works for at least 24 hours. If you did not ask for this, ignore this email and nothing will change.
";

		if( !mail( $newEmail, "confirm your new [this might be offensive] email address", $message, "From: offensive@thismight.be (this might be offensive)" ) ) {
			trigger_error( "could not send email change confirmation", E_USER_WARNING );
			return new statusMessage( false, "Couldn't send the confirmation email. Try again later." );
		}

		$_SESSION['email_change_sent'] = time();
		return new statusMessage( true, "Sent. Follow the link in the email to $newEmail to finish the change." );
	}

	/* handle a confirmation link.  returns a statusMessage. */
	function emailChangeConfirm( $code, $newEmail )
	{
		$bad = new statusMessage( false, "That confirmation link is expired or has already been used." );

		if( TMBO::readonly() ) {
			return new statusMessage( false, "This session is read-only." );
		}

		$code = (string)$code;
		$newEmail = (string)$newEmail;

		if( id_from_hash( $code ) != me()->id() ) {
			return new statusMessage( false, "That confirmation link is for a different account. Log in as that account and follow it again." );
		}

		$row = emailChangeUserRow( me()->id() );
		if( !$row ) {
			return $bad;
		}

		$today = gmdate( "Y-m-d" );
		$yesterday = gmdate( "Y-m-d", time() - 86400 );
		if( !tmbo_hash_equals( emailChangeCode( $row, $newEmail, $today ), $code ) &&
		    !tmbo_hash_equals( emailChangeCode( $row, $newEmail, $yesterday ), $code ) ) {
			return $bad;
		}

		// again, in case another account took the address since the email was sent.
		$valid = emailChangeValidate( $row, $newEmail );
		if( !$valid->status ) {
			return $valid;
		}

		$uid = (int)$row['userid'];
		$old = (string)$row['email'];
		// the email clause keeps two clicks of the same link from both reporting success.
		tmbo_query( "UPDATE users SET timestamp = timestamp, email = '" . sqlEscape( $newEmail ) . "'
		             WHERE userid = $uid AND email <=> " . ( is_null( $row['email'] ) ? "NULL" : "'" . sqlEscape( $old ) . "'" ) . " LIMIT 1" );
		global $link;
		if( mysqli_affected_rows( $link ) != 1 ) {
			return $bad;
		}

		// tell the old address, so a takeover is not silent.  a dead old address is
		// the main reason anyone uses this, so failing to send is not an error.
		if( isValidEmailFormat( $old ) ) {
			$message = "The email address for the [this might be offensive] account \"" . $row['username'] . "\" was changed from $old to $newEmail.

If you did this, there's nothing else to do.

If you didn't, someone else has your password. Contact us at https://thismight.be/contact/ to get the account back.
";
			mail( $old, "your [this might be offensive] email address was changed", $message, "From: offensive@thismight.be (this might be offensive)" );
		}

		return new statusMessage( true, "Your email address is now $newEmail." );
	}

?>
