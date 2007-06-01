<?

	require_once( "activationFunctions.php" );
	require_once( "classes/statusMessage.php" );

	$message;

	function start() {
		
		global $message;

		$username = trim( $_POST['username'] );
		$password = trim( $_POST['password'] );
		$referral_code = trim( $_POST['referral_code'] );

		$accountCreated = false;

		if( $_REQUEST['username'] != "" ) {
			$message = handleAccountRequest( $username, $password, $referral_code );
		}
	}
	
	function noLogin() {
		// the existence of this function
		// tells the template not to include
		// the login form.
	}

	function handleAccountRequest($username, $password, $referral_code) {

		$emailValidation = isValidEmail( $_REQUEST['email'] );

		if( ! $emailValidation->status ) {
			return $emailValidation;
		}

		if( isValidUsername( $username )
				&& isValidPassword( $password )
				&& $username != $password 
				&& isValidReferral( $referral_code )
				)
		{
			
			$result = createAccount( $username, $password, $referral_code );
			if( $result == "OK" ) {
				$message = new statusMessage( true, "Account created." );
				$accountCreated = true;
			} else {
				$message = new statusMessage( false, $result );
			}
			
		} else {
			$message = new statusMessage( false, "Invalid username or password. Passwords must be at least 5 characters long and may consist of letters, numbers, underscores, periods, and dashes. Passwords must not be the same as your username." );
		}
		
		return $message;
	
	}


	function isValidReferral( $code ) {
		$sql = "select * from referrals where referral_code = '$code'";
		$result = mysql_query( $sql );
		return (mysql_num_rows( $result ) > 0);
	}

	function isValidEmail( $email ) {
	
		$valid = preg_match( '/[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+/', $email ) > 0
		&& strpos( $email, "hotmail.com" ) == false
		&& strpos( $email, "mailinator.com" ) == false		
		&& strpos( $email, "yahoo.com" ) == false;

		if( ! $valid ) {
			return new statusMessage( false, "The email address provided is invalid, either because it contains structural errors or is from an unsupported provider (yahoo, hotmail, etc)." );
		}
	
	    $link = openDbConnection();
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

	
	function createAccount( $uName, $pw, $email, $referral ) {
	
		$returnMessage = "OK";
		
	    // Include, and check we've got a connection to the database.
		include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();
		
	    $query = "SELECT count(*) AS theCount FROM users WHERE username = '" . $uName . "'";

	    $result = mysql_query($query) or die("Query failed");

		// get the results of the query as an associative array, indexed by column name
		$row = mysql_fetch_array( $result, MYSQL_ASSOC );
		
		if( $row['theCount'] == 0 ) {

			$sql = "select * from referrals where referral_code = '$referral'";

			$result = mysql_query( $sql );
			$row = mysql_fetch_assoc( $result );
			$referrer = $row['userid'];

			$encrypted_pw = sha1( $pw );
		
			$query = "INSERT INTO users (username,password,email,referred_by,created,ip) VALUES ( '$uName','$encrypted_pw', '$email', $referrer, now(), '" . $_SERVER['REMOTE_ADDR']. "' )";
			$result = mysql_query($query) or die("Query failed"); 
			$result = mysql_query("SELECT userid,account_status from users where username = '$uName'") or die("Query failed"); 
			$row = mysql_fetch_array( $result, MYSQL_ASSOC );
			if( $row['account_status'] == 'normal' ) {
				$_SESSION['userid'] = $row['userid'];
				$_SESSION['username'] = $uName;
			}
			
			$activationMessage = activationMessageFor( $row['userid'], $_POST['email'] );
			
			mail( $_POST['email'], "themaxx.com account activation", "$activationMessage", "From: offensive@themaxx.com (this might be offensive)");
#			mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - themaxx.com account created: $uName", $_POST['email'], "From: offensive@themaxx.com (this might be offensive)");
		} else {
		 	$returnMessage = "The username you've chosen, \"" . $uName . "\", is not available.";
		}
		
	    /* Closing connection */
	    mysql_close($link);
	    
    	return $returnMessage;
	}
	
	
	function body() {
		global $message;
?>
		<div class="heading">sign me up.</div>
		<div class="bluebox">

			<?= $message->message ?>

			<form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
				<input type="hidden" name="c" value="register" />

				<div class="entry odd_row">
					desired username:<br/>
					<input type="text" name="username" size="50" value="<?= $_REQUEST['username'] ?>" />
				</div>
				
				<div class="entry even_row">
					password:<br/>
					<input type="password" name="password" size="50" />
				</div>
				<div class="entry odd_row">
					repeat password:<br/>
					<input type="password" name="password2" size="50" />
				</div>
				<div class="entry even_row">
					email: (activation instructions will be sent to this address. you will <b>not</b> be spammed.)<br/>
					<input type="text" name="email" size="50" value="<?= $_REQUEST['email'] ?>"/>
				</div>
				<div class="entry odd_row">
					invitation code:<br/>
					<input type="text" name="referral_code" size="50" value="<?= $_REQUEST['referral_code'] ?>"/>
				</div>
				<div class="entry even_row">
					<input type="submit" value="go!">
				</div>
			</form>
		</div>
<?
	}
?>