<?php
	session_start();

# assume this will be included by the page that includes login.php
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'getPrefs.php' );
	require_once( 'activationFunctions.php' );

	$login_message = "";

	$redirect = $_REQUEST['redirect'];
	if( $redirect == null ) {
		$redirect = $_SERVER['HTTP_REFERER'];
	}

	if( $_REQUEST['password'] ) {
		$success = login( $_REQUEST['username'], $_REQUEST['password'] );
		if( $success === true && $_REQUEST['rememberme'] ) {
			setcookie( "remember", tmbohash( $_SESSION['userid'], $_SESSION['username'] . $_SERVER['REMOTE_ADDR'] . $salt ), time()+60*60*24*365*5, "/" );
		}
		header( "Location: ./" );
	}

	if( ! isset( $_SESSION['userid'] ) ) {
		$rememberCookie = $_COOKIE['remember'];
		if( isset( $rememberCookie ) ) {
			if( loginFromCookie( $rememberCookie ) ) {
				header( "Location: ./" );
			}
		}
	}
	
	function loginFromCookie( $cookieValue ) {
		
		global $salt;
	
		$uid = id_from_hash( $cookieValue );
		
		if( is_numeric( $uid ) ) {

			$link = openDbConnection();
			$sql = "SELECT * from users where userid=$uid LIMIT 1";
			$result = mysql_query( $sql );
			if( mysql_num_rows( $result ) == 1 ) {
				$row = mysql_fetch_assoc( $result );
				$cookiehash = tmbohash( $row['userid'], $row['username'] . $_SERVER['REMOTE_ADDR'] . $salt );
				if( $cookiehash == $cookieValue ) {
					$sql = "SELECT userid, username, account_status FROM users WHERE userid=$uid";
					$result = mysql_query( $sql );
					return loginFromQueryResult( $result );
				}
			}
			
		}
	
	}

	function loginFromQueryResult( $result ) {

		global $login_message;

		if( mysql_num_rows($result) > 0 ) {		
			$row = mysql_fetch_assoc( $result );
			$status = $row['account_status'];
			$uid = $row['userid'];

			if( is_numeric( $uid ) ) {
				$sql = "UPDATE users SET last_login_ip='" . $_SERVER['REMOTE_ADDR'] . "', timestamp=now() WHERE userid=$uid LIMIT 1";
				mysql_query( $sql );
			}

			if( $status == 'normal' ) {
				$_SESSION['userid'] = $row['userid'];
				$_SESSION['username'] = $row['username'];
				$prefs = getPreferences( $row['userid'] );
				$_SESSION['prefs'] = $prefs;
				return true;
			}
			else if( $status == 'locked' ) {
				$login_message = "<b>That account is locked.</b>";
			} else if( $status == 'awaiting activation' ) {
				$login_message = "<b>That account is awaiting activation.</b>";
			}
		}
		else {
			session_unset();
		}
	
	}

	function logIn( $name, $pw ) {
		
		// values defined in mysqlConnectionInfo.php
		global $db_url, $db_user, $db_pw;

		$link = openDbConnection();
	
        $encrypted_pw = sha1( $pw );
		
		$query = "SELECT userid, username, account_status FROM users WHERE username = '" . $name . "' AND password = '" . $encrypted_pw . "'";

		$result = mysql_query($query) or die("Login query failed." );
	
		return loginFromQueryResult( $result );

	}

?>

<span class="small">

<?php echo $login_message ?>

<?php if( isset($_SESSION['userid']) ) { ?>

	<p>you are logged in as <b><a href="index.php?c=user&userid=<?php echo $_SESSION['userid'] ?>"><?php echo $_SESSION['username'] ?></a>.</b></p>
	
	<p><a href="index.php?c=upload">upload</a></p>

	<p><a href="logout.php">log out</a></p>
		
<?php
	} else {
?>

	<form action="./login.php" method="post">
		<input type="hidden" name="redirect" value="<?php echo $redirect ?>"/>
		user name:<br/><input type="text" name="username" size="12"/><br/>
		password:<br/><input type="password" name="password" size="12"/><br/>
		<input type="checkbox" name="rememberme" id="rememberme" value="1"><label for="rememberme">remember me</label><br/>
		<input type="submit" value="log in"/>
	</form>
	<br>
	<a href="register.php">register</a>

<?php
	}
?>
</span>
