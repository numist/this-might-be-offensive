<?
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once( 'offensive/assets/functions.inc' );
require_once('offensive/assets/activationFunctions.inc');

global $prompt, $login_message, $login_called;
$prompt = true;
$login_message = "";
$login_called = false;

/* 
 * this function ONLY returns if the user is logged in.  
 * if the user is not logged in (or fails to authenticate, in the case of HTTP
 * auth), mustLogIn will terminate the script.
 */
function mustLogIn($methods=array()) {
	if(login($methods)) {
		return;
	}
	
	ob_end_clean();
	session_unset();
	
	global $prompt, $login_message;
	
	if(array_key_exists("prompt", $methods) && $methods["prompt"] == "http") {
		// if we have credentials, try them and either fail or return.
		if (isset($_SERVER['PHP_AUTH_USER']) && strlen($_SERVER['PHP_AUTH_USER']) > 0) {
			$methods = array("u/p" => array($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']));
			if(login($methods) === true) return;

			header('HTTP/1.0 401 Not Found');
    	echo $login_message;
			exit;
		}

		// no auth? try again.
		if($prompt) {
			header('WWW-Authenticate: Basic realm="[this might be offensive]"');
		}
    header('HTTP/1.0 401 Unauthorized');
		if(strlen($login_message) == 0) {
    	echo 'anon is not welcome here.';
		} else {
			echo $login_message;
		}
    exit;
	} else {
		// build the redirect link
		$redirect = (isset($_REQUEST['redirect']) ?
		            $_REQUEST['redirect'] :
		            "https://".$_SERVER["SERVER_NAME"].urlencode($_SERVER['REQUEST_URI']));

		header("Location: https://".$_SERVER["SERVER_NAME"]."/offensive/logn.php?redirect=".$redirect);
		exit;
	}
}

/*
 * check the status of the user and fail if the user is not normal or admin.
 * Side Effects:
 * 	may set globals $login_message and $prompt:
 *  * $login_message is an error message to display to the user.
 *  * $prompt is whether or not the user is allowed to log in 
 *            (whether a login prompt should be shown)
 */
function checkUser($me) {
	global $login_message, $prompt;

	if($me->status() == "normal" || $me->status() == "admin") {
		$GLOBALS["me"] = $me;
		return true;
	}

	if($me->status() == 'locked') {
		$login_message = "that account is locked.";
	} else if( $status == 'awaiting activation' ) {
		$login_message = "that account is awaiting activation.";
	}
	$prompt = false;
	session_unset();
	return false;
}

/*
 * logInUser sets up the session and enters the successful login into ip_history
 * this function does not do any authentication, it is just a shared resource used by various 
 * authentication mechanisms to process a successful login attempt.
 */
function logInUser($me) {
	global $login_message, $prompt;

	if(session_id() == "") {
		session_start();
	}

	$sql = "UPDATE users SET last_login_ip='" . $_SERVER['REMOTE_ADDR'] . "', timestamp=NOW() WHERE userid=".$me->id()." LIMIT 1";
	tmbo_query( $sql );

	if( $me->status() == 'normal' || $me->status() == 'admin' ) {
		$_SESSION['userid'] = $me->id();
		$_SESSION['status'] = $me->status();
		$_SESSION['username'] = $me->username();
			
		$sql = "INSERT INTO ip_history (userid, ip) VALUES ( ".$me->id().", '".$_SERVER['REMOTE_ADDR']."' )";
		tmbo_query( $sql );
	}
	
	return checkUser($me);
}

/*
 * is this host allowed to make an authentication attempt?
 */
function canLogIn() {
	// check to see if this person has had too many login attempts in the last 30 minutes already.
	$ip = $_SERVER['REMOTE_ADDR'];
	$sql = "SELECT count( id ) as numFailed from failed_logins WHERE ip='$ip' AND timestamp > date_sub( now(), interval 30 minute )";
	$result = tmbo_query( $sql );

	$row = @mysql_fetch_assoc( $result );

	if( $row['numFailed'] > 5 ) {
		return false;
	}
	return true;
}

/*
 * log a potentially-malicious failure of credentials.
 * this is used to determine if a host canLogIn.
 */
function failedLogIn($name) {
	$ip = $_SERVER['REMOTE_ADDR'];
	$sql = "insert into failed_logins (username,ip) VALUES ( '".sqlEscape($name)."', '$ip' )";
	tmbo_query( $sql );
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
function login($methods=array()) {
	global $login_called;
	$login_called = true;
	
	assert('is_array($methods)');
	global $login_message;

	// session login is always allowed!
	$success = loginWithSession();
	if(!is_null($success)) {
		return $success;
	}

	// cookie login is always allowed!
	$success = loginWithCookie();
	if(!is_null($success)) {
		return $success;
	}
	
	/*
	 * the methods below are all non-automated, so check to see if this IP has
	 * been annoying lately.
	 */
	if(!canLogIn()) {
		$login_message = "give it a rest.";
		return null;
	}
	
	foreach($methods as $method => $params) {
		switch($method) {
			case "u/p":
				$success = loginWithUserPass($params);
				if(!is_null($success)) {
					return $success;
				}
				break;
			case "token":
				$success = loginWithToken($params);
				if(!is_null($success)) {
					return true;
				}
				break;
		}
	}
	return false;
}

/*
 * attempt to log on using the session as credentials.
 * returns a boolean if success/failure, null if not applicable.
 */
function loginWithSession() {
	if(array_key_exists('userid', $_SESSION) && is_intger($_SESSION['userid'])) {
		$me = new User($_SESSION["userid"]);
		return checkUser($me);
	}
	return null;
}

/*
 * attempt to log on using the rememberme cookie as credentials.
 * returns a boolean if success/failure, null if not applicable.
 */
function loginWithCookie() {
	// attempt logging in using the user's cookie
	if(isset($_COOKIE['remember'])) {
		global $salt;

		$uid = id_from_hash( $_COOKIE['remember'] );
		if(is_intger($uid)) {
			$sql = "SELECT * from users where userid=$uid LIMIT 1";
			$result = tmbo_query( $sql );
			if(mysql_num_rows( $result ) == 1) {
				$row = mysql_fetch_assoc($result);
				$cookiehash = tmbohash($row['userid'], $row['username'].$salt);
				if($cookiehash == $_COOKIE['remember']) {
					return logInUser(new User($row));
				}
			}
		}
	}
	return null;
}

/*
 * attempt to log on using a supplied username and password as credentials.
 * returns a boolean if success/failure, null if not applicable.
 */
function loginWithUserPass($params) {
	global $login_message, $salt;

	if(count($params) == 2 && !is_null($params[0]) && !is_null($params[1])) {
		list($name, $pw) = $params;

		// attempt login using u/p, if available
   	$encrypted_pw = sha1( $pw );
		$query = "SELECT userid FROM users WHERE username = '" . sqlEscape($name) . "' AND password = '" . $encrypted_pw . "'";
		$result = tmbo_query($query);
		if(mysql_num_rows($result) > 0) {
			$userid = array_top(mysql_fetch_assoc($result));

			if(logInUser(new User($userid))) {
				// set rememberme cookie if the user requested it
				if(array_key_exists("rememberme", $_REQUEST) && $_REQUEST['rememberme']) {
					setcookie("remember", 
					          tmbohash(me()->id(), 
					                   me()->username().$salt ), 
					                   time()+60*60*24*365*5, "/" );
				}
				return true;
			}
			return false;
		}

		// log a failure if a username and password did not result in success
		// this only applies to people not using http_auth for now, due to some weird client bugs locking people out.
		if(!isset($_SERVER["PHP_AUTH_USER"]) || strlen($_SERVER["PHP_AUTH_USER"]) == 0) {
			failedLogIn($name);
		}
		// if the credentials were wrong (as opposed to being snubbed or banned) set a helpful message for the user
		$login_message = '<a href="/offensive/pwreset.php">tried resetting your password?</a>';
		
		return false;
	}
	return null;
}

/*
 * attempt to log on using a token in the request as credentials.
 * returns a boolean if success/failure, null if not applicable.
 */
function loginWithToken($nothing) {
	$tokenid = array_key_exists("token", $_REQUEST) ? $_REQUEST["token"] : "";

	if(strlen($tokenid)) {
		$token = new Token($tokenid);
		if(!$token->exists()) {
			//failedLogIn("");
			return false;
		}
		
		return logInUser(new User(array("userid" => $token->userid())));
	}
	return null;
}

?>