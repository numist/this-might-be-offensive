<?
/*
 * loggedin() and mustLogIn() can be found in header.inc as they are more widely used and do not need
 * the library inclusions that other functions in this file require.
 */

// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once( 'offensive/assets/functions.inc' );
require_once('offensive/assets/activationFunctions.inc');

/*
 * loginFromQueryResult logs in a user from a mysql resource containing a row from the users table.
 * this function does not do any authentication, it is just a shared resource used by various 
 * authentication mechanisms to process a successful login attempt.
 * Side Effects:
 * 	may set $login_message and $prompt:
 *  * $login_message is an error message to display to the user.
 *  * $prompt is whether or not the user is allowed to log in 
 *            (whether a login prompt should be shown)
 * Notes:
 *  * this should never be called from outside this file!
 */
function loginFromQueryResult( $result ) {
	global $login_message, $prompt;

	if( mysql_num_rows($result) > 0 ) {		
		$row = mysql_fetch_assoc( $result );
		$status = $row['account_status'];
		$uid = $row['userid'];

		$sql = "UPDATE users SET last_login_ip='" . $_SERVER['REMOTE_ADDR'] . "', timestamp=NOW() WHERE userid=$uid LIMIT 1";
		tmbo_query( $sql );

		if( $status == 'normal' || $status == 'admin' ) {
			$_SESSION['userid'] = $row['userid'];
			$_SESSION['status'] = $status;
			$_SESSION['username'] = $row['username'];
			
			$sql = "INSERT INTO ip_history (userid, ip) VALUES ( ".$row['userid'].", '".$_SERVER['REMOTE_ADDR']."' )";
			tmbo_query( $sql );
			
			return true;
		}
		else if( $status == 'locked' ) {
			$login_message = "that account is locked.";
			$prompt = false;
		} else if( $status == 'awaiting activation' ) {
			$login_message = "that account is awaiting activation.";
			$prompt = false;
		}
	}
	else {
		session_unset();
	}

}

/* Returns:
 * true: user successfully logged in somehow (either by _REQUEST or _COOKIE)
 * false: user failed to log in (bad password, no cookie) no password or cookie will also cause this
 * null: auth attempt from host (not username!) ignored -- usually due to flood limiting
 * Side Effects (from loginFromQueryResult and self):
 * 	may set $login_message and $prompt:
 *  * $login_message is an error message to display to the user.
 *  * $prompt is whether or not the user is allowed to log in 
 *            (whether a login prompt should be shown)
 */
function login( $name=null, $pw=null ) {
	if(strlen($name) == 0) $name = null;
	if(strlen($pw) == 0) $pw = null;
	
	global $login_message, $salt;
	
	// strict sanity check
	if(loggedin(false)) return true;

	if($name !== null && $pw !== null) {
		// check to see if this person has had too many login attempts in the last 30 minutes already.
		$ip = $_SERVER['REMOTE_ADDR'];
		$sql = "SELECT count( id ) as numFailed from failed_logins WHERE ip='$ip' AND timestamp > date_sub( now(), interval 30 minute )";
		$result = tmbo_query( $sql );

		$row = @mysql_fetch_assoc( $result );

		if( $row['numFailed'] > 5 ) {
			$login_message = "give it a rest.";
			return null;
		}

		// attempt login using u/p, if available
    	$encrypted_pw = sha1( $pw );
		$query = "SELECT userid, username, account_status FROM users WHERE username = '" . sqlEscape($name) . "' AND password = '" . $encrypted_pw . "'";

		$result = tmbo_query($query);

		// set rememberme cookie if the user requested it
		if(mysql_num_rows($result) > 0 && loginFromQueryResult( $result ) === true) {
			if(array_key_exists("rememberme", $_REQUEST) && $_REQUEST['rememberme'])
				setcookie("remember", 
				          tmbohash($_SESSION['userid'], 
				                   $_SESSION['username'].$salt ), 
				                   time()+60*60*24*365*5, "/" );
			return true;
		}

		// log a failure if a username and password did not result in success
		// this only applies to people not using http_auth for now, due to some weird client bugs locking people out.
		if(!isset($_SERVER["PHP_AUTH_USER"]) || strlen($_SERVER["PHP_AUTH_USER"]) == 0) {
			$ip = $_SERVER['REMOTE_ADDR'];
			$sql = "insert into failed_logins (username,ip) VALUES ( '".sqlEscape($name)."', '$ip' )";
			tmbo_query( $sql );
        	
			$sql = "select count(id) as thecount from failed_logins where ip='$ip' and timestamp > date_sub( now(), interval 1 day )";
			$result = tmbo_query( $sql );
			$row = mysql_fetch_assoc( $result );
			$count = $row['thecount'];
			if( $count > 3 && isset( $_REQUEST['username'] ) && $_REQUEST['username'] != null && $_REQUEST['username'] != '') {
				ob_start();
				echo "Username on this attempt: " . $_REQUEST['username'] . "\n\n";
				var_dump( $_SERVER );
				var_dump( $_REQUEST );		
				$string = ob_get_contents();
				ob_end_clean();
					
				mail( "ray@sneakymeans.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - $count FAILED LOGIN ATTEMPTS TODAY!!! ", $string, "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			}
		}
	}
	
	// last chance -- log in using the user's cookie
	// this is the only check that happens if the call did not include a username and password.
	if(isset($_COOKIE['remember'])) {
		global $salt;

		$uid = id_from_hash( $_COOKIE['remember'] );
		if(is_numeric($uid)) {
			$sql = "SELECT * from users where userid=$uid LIMIT 1";
			$result = tmbo_query( $sql );
			if(mysql_num_rows( $result ) == 1) {
				$row = mysql_fetch_assoc($result);
				$cookiehash = tmbohash($row['userid'], $row['username'].$salt);
				if($cookiehash == $_COOKIE['remember']) {					
					$sql = "SELECT userid, username, account_status FROM users WHERE userid=$uid";
					$result = tmbo_query( $sql );
					return loginFromQueryResult( $result );
				}
			}
		}
	}
	
	// if the login failed (as opposed to being snubbed) set a helpful message for the user
	if($pw && strlen($login_message) == 0) {
		$login_message = '<a href="/offensive/pwreset.php">tried resetting your password?</a>';
	}
	
	return false;
}

?>