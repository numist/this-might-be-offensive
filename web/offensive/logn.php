<?php
	session_start();

	$redirect = $_REQUEST['redirect'];
	if( $redirect == null ) {
		$redirect = './';
	}

	if( is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: " . $redirect );
		exit;
	}

	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'getPrefs.php' );
	require_once( 'activationFunctions.php' );

	$login_message = "";


	if( $_REQUEST['password'] ) {
		$success = login( $_REQUEST['username'], $_REQUEST['password'] );
		if( $success === true && $_REQUEST['rememberme'] ) {
			setcookie( "remember", tmbohash( $_SESSION['userid'], $_SESSION['username'] . $_SERVER['REMOTE_ADDR'] . $salt ), time()+60*60*24*365*5, "/" );
		}
		if( $success === true ) {
			header( "Location: " . $redirect );
			exit;
		}
		else {
			logAttempt();
		}
	}

	if( ! isset( $_SESSION['userid'] ) ) {
		$rememberCookie = $_COOKIE['remember'];
		if( isset( $rememberCookie ) ) {
			if( loginFromCookie( $rememberCookie ) ) {
				header( "Location: " . $redirect );
				exit;
			}
		}
	}

	function logIp( $uid ) {
		$link = openDbConnection();
		$ip = $_SERVER['REMOTE_ADDR'];
		$sql = "INSERT INTO ip_history (userid, ip) VALUES ( $uid, '$ip' )";
		$result = mysql_query( $sql, $link );

	}

	function logAttempt() {
		global $login_message;
	
		$uname = mysql_real_escape_string( $_REQUEST['username'] );
		$pw = mysql_real_escape_string( $_REQUEST['password'] );
		$ip = $_SERVER['REMOTE_ADDR'];
		$sql = "insert into failed_logins (username,ip) VALUES ( '$uname', '$ip' )";
		@mysql_query( $sql );
		
		$sql = "select count(id) as thecount from failed_logins where ip='$ip' and timestamp > date_sub( now(), interval 1 day )";
		$result = mysql_query( $sql );
		echo mysql_error();
		$row = mysql_fetch_assoc( $result );
		$count = $row['thecount'];
		if( $count > 3 ) {
			mail( "ray@sneakymeans.com,jerrywilborn@gmail.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - $count FAILED LOGIN ATTEMPTS TODAY!!! ", requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
			$login_message = '<a href="./pwreset.php">forgot your password?</a>';
		}	

	}
	
	function requestDetail() {
		ob_start();
		echo "Username on this attempt: " . $_REQUEST['username'] . "

";
		var_dump( $_SERVER );
		var_dump( $_REQUEST );		
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
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

			if( $status == 'normal' || $status == 'admin' ) {
				$_SESSION['userid'] = $row['userid'];
				$_SESSION['status'] = $status;
				$_SESSION['username'] = $row['username'];
				$prefs = getPreferences( $row['userid'] );
				$_SESSION['prefs'] = $prefs;
				logIp( $row['userid'] );
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
		
		$ip = $_SERVER['REMOTE_ADDR'];
		$sql = "SELECT count( id ) as numFailed from failed_logins WHERE ip='$ip' AND timestamp > date_sub( now(), interval 30 minute )";
		$result = @mysql_query( $sql );
	
		$row = @mysql_fetch_assoc( $result );
		
		if( $row['numFailed'] > 5 ) {
			echo "give it a rest.";
			exit;
		}

        $encrypted_pw = sha1( $pw );
		
		$query = "SELECT userid, username, account_status FROM users WHERE username = '" . $name . "' AND password = '" . $encrypted_pw . "'";

		$result = mysql_query($query) or die("Login query failed." );
	
		return loginFromQueryResult( $result );

	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">

<html>
<head>
	<title>themaxx.com : do we know you?</title>
	<link rel="stylesheet" type="text/css" href="/styles/sparse.css"/>
</head>


<body bgcolor="#ffffff">

	<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
		<tr>
			<td valign="center" height="100%" align="center">
	
				<p>if you haven't <a href="pwreset.php">reset your password</a> yet, you won't be able to log in.</p>
	
				<p>

				<span class="small">
					<form action="./logn.php" method="post">

						<table>
							<tr>
								<td colspan="2"><?php echo $login_message ?></td>
							</tr>
							<tr>
								<td class="label">user name:</td>
								<td><input type="text" name="username" size="12"/></td>
							</tr>
							<tr>
								<td class="label">password:</td>
								<td><input type="password" name="password" size="12"/></td>
							</tr>
							<tr>
								<td class="label"></td>
								<td><input type="checkbox" name="rememberme" id="rememberme" value="1"><label for="rememberme">remember me</label><br/></td>
							</tr>
							<tr>
								<td colspan="2" class="submitcell">
									<input type="submit" class="button" value="log in"/>
								</td>
							</tr>
						</table>
					</form>
				
				</span>

				</p>
	
			</td>
		</tr>
	</table>
	
<? include '../includes/footer.txt' ?>

</body>
</html>
