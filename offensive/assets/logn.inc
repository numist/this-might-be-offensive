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
			/* this is technically misuse of HTTP code 401 because it does not
			 * include a WWW-Authenticate header, but it is consistent with the
			 * HTTP error code expectations in the API.
			 */
			require("offensive/401.php");
		}

		// no auth? try again.
		if($prompt) {
			header('WWW-Authenticate: Basic realm="[this might be offensive]"');
			require("offensive/401.php");
		} else {
			require("offensive/503.php");
		}
	} else {
		// build the redirect link
		$redirect = (isset($_REQUEST['redirect']) ?
		            $_REQUEST['redirect'] :
		            "https://".$_SERVER["HTTP_HOST"].urlencode($_SERVER['REQUEST_URI']));

		header("Location: https://".$_SERVER["HTTP_HOST"]."/offensive/logn.php?redirect=".$redirect);
	}
	exit;
}

/*
 * check the status of the user and fail if the user is not normal or admin.
 * Side Effects:
 * 	may set globals $login_message and $prompt:
 *  * $login_message is an error message to display to the user.
 *  * $prompt is whether or not the user is allowed to log in 
 *            (whether a login prompt should be shown)
 */
function checkUser($myself) {
	global $login_message, $prompt;

	if(!$myself->exists()) return false;

	if($myself->status() == "normal" || $myself->status() == "admin") {
		// don't update the user's row unless the IP has changed or it's been a minute since the last update.
		if(strtotime($myself->timestamp()) < time() - 60 ||
		   $myself->last_login_ip() != $_SERVER['REMOTE_ADDR']) {
			
			$sql = "UPDATE users SET last_login_ip='" . $_SERVER['REMOTE_ADDR'] . "', timestamp=NOW() WHERE userid=".$myself->id();
			tmbo_query( $sql );
			
			// update the User cache with the new data.
			$args = array("userid" => $myself->id(),
			              "last_login_ip" => $_SERVER['REMOTE_ADDR'],
			              "timestamp" => date("Y-m-d H:i:s"));
			new User($args);
		}
		
		$GLOBALS["me"] = $myself;
		return true;
	}

	if($myself->status() == 'locked') {
		$login_message = "that account is locked.";
	} else if( $myself->status() == 'awaiting activation' ) {
		$login_message = "that account is awaiting activation.";
	}
	$prompt = false;
	session_unset();
	return false;
}

/*
 * logInUser sets up the session
 * this function does not do any authentication, it is just a shared resource used by various 
 * authentication mechanisms to process a successful login attempt.
 */
function logInUser($myself, $session=true) {
	global $login_message, $prompt;

	if( $myself->status() == 'normal' || $myself->status() == 'admin') {
		if($session) {
			if(session_id() == "") {
				// must match header.inc; see the note there.  guarded because
				// session_name() warns if a session is already active.
				session_name("TMBOSESS2");
				session_set_cookie_params(0, "/", "", true, true);
				session_start();
			}
			$_SESSION['userid'] = $myself->id();
			$_SESSION['status'] = $myself->status();
			$_SESSION['username'] = $myself->username();
	   } else {
			session_unset();
	   }
	}
	
	return checkUser($myself);
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

	/* Avoid unnecessary work
	 * Sometimes login is called once without any arguments in case a session or
	 * cookie is lying around and later with actual credentials. Check both that
	 * this is a duplicate call *and* that a user is actually logged in before
	 * short-circuiting.
	 */
	if($login_called && me()) {
		return true;
	}
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
		global $login_message, $prompt;
		$login_message = "give it a rest.";
		$prompt = false;
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
	if(isset($_SESSION) &&
	   array_key_exists('userid', $_SESSION) &&
	   is_intger($_SESSION['userid'])) {
		$myself = new User($_SESSION["userid"]);
		return checkUser($myself);
	}
	return null;
}

/*
 * the rememberme cookie.
 *
 * the cookie is "userid.nonce.mac", where the mac is an HMAC over the first two
 * fields, keyed with the user's stored password hash concatenated with a
 * site-wide pepper from admin/.config.
 *
 * this replaces a scheme that hashed only the userid, the username, and a salt
 * that was a literal in the public repository -- every input was public, so
 * anyone could compute a valid cookie for any account without ever touching the
 * site.  it is worth spelling out what each piece here is for:
 *
 *   * the password hash means a cookie cannot be derived from public data, and
 *     that changing a password invalidates that user's cookies as a consequence
 *     of the scheme rather than of remembering to revoke them.  pwreset.php
 *     keys its reset codes the same way and for the same reason.
 *   * the pepper means a dump of the users table is not by itself enough to mint
 *     cookies; you need the server's config file too.  rotating it invalidates
 *     every cookie on the site at once, without writing to the database -- which
 *     is the only invalidation lever that works while the site is read-only.
 *   * the nonce makes each issued cookie unique, so the cookie is not a
 *     deterministic function of the account and two devices don't share a value.
 *
 * NOTE: revocation is all-or-nothing across a user's devices, and the only
 * per-user lever is a password change.  per-device revocation would need a
 * separate token table.
 */
define("REMEMBER_COOKIE", "remember");
define("REMEMBER_TTL", 60*60*24*365*5);

function rememberCookieMac($userid, $nonce, $pwhash) {
	return hash_hmac("sha256", $userid.".".$nonce, $pwhash.tmbo_secret("remember_pepper"));
}

/*
 * issue a fresh rememberme cookie for a user.
 *
 * this reads and never writes, so it behaves correctly under TMBO::readonly() --
 * tmbo_query() silently discards non-SELECT queries there, so a scheme that had
 * to store per-user state would mint a cookie keyed on state that was never
 * persisted, and the cookie would never verify.
 */
function issueRememberCookie($myself) {
	$uid = $myself->id();

	$sql = "SELECT password FROM users WHERE userid=$uid LIMIT 1";
	$result = tmbo_query( $sql );
	if(mysql_num_rows( $result ) != 1) {
		trigger_error("tried to issue a remember cookie for nonexistent user $uid", E_USER_WARNING);
		return false;
	}

	$row = mysql_fetch_assoc( $result );

	/* users.password is nullable, and a null hash would leave the pepper as the
	 * whole key -- shared by every such account.  no hash, no cookie.
	 */
	if(is_null($row['password']) || strlen($row['password']) == 0) {
		trigger_error("tried to issue a remember cookie for passwordless user $uid", E_USER_WARNING);
		return false;
	}

	$nonce = tmbo_random_hex(16);
	$value = $uid.".".$nonce.".".rememberCookieMac($uid, $nonce, $row['password']);

	// secure: this site is https-only (see header.inc).  httponly: script has no business reading this.
	setcookie(REMEMBER_COOKIE, $value, time() + REMEMBER_TTL, "/", "", true, true);
	return true;
}

/*
 * attempt to log on using the rememberme cookie as credentials.
 * returns a boolean if success/failure, null if not applicable.
 */
function loginWithCookie() {
	// attempt logging in using the user's cookie
	if(isset($_COOKIE[REMEMBER_COOKIE])) {
		$parts = explode(".", $_COOKIE[REMEMBER_COOKIE]);
		if(count($parts) != 3) {
			return null;
		}
		list($uid, $nonce, $mac) = $parts;

		/* ctype_digit rather than is_intger: this field is attacker-controlled,
		 * and is_intger() accepts anything is_numeric() likes -- "1e3", " 1",
		 * "1.0".  none of those are exploitable here, because the mac is keyed
		 * on the userid that comes back from the database rather than on this
		 * string, but "1e3" casts to 1 on php 5 and 1000 on php 7, and a cookie
		 * parser is no place to leave that lying around.
		 */
		if(ctype_digit($uid) && strlen($nonce) && strlen($mac)) {
			$uid = (int)$uid;
			$sql = "SELECT * from users where userid=$uid LIMIT 1";
			$result = tmbo_query( $sql );
			if(mysql_num_rows( $result ) == 1) {
				$row = mysql_fetch_assoc($result);

				/* no stored hash means there is nothing to key the mac with, so
				 * there is nothing to verify against.  see issueRememberCookie().
				 */
				if(!is_null($row['password']) && strlen($row['password'])) {
					$cookiemac = rememberCookieMac($row['userid'], $nonce, $row['password']);
					if(tmbo_hash_equals($cookiemac, $mac)) {
						return logInUser(new User($row));
					}
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
	global $login_message;

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
					issueRememberCookie(me());
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
			return null;
		}
		
		// don't update the token's row unless it's been a minute since the last update.
		if(strtotime($token->last_used()) < time() - 60) {
			$sql = "UPDATE tokens SET last_used = NOW() WHERE tokenid = \"".$token->tokenid()."\"";
			tmbo_query($sql);
		}
		
		TMBO::readonly($token->readonly());
		
		return logInUser(new User(array("userid" => $token->userid())), false);
	}
	return null;
}

?>