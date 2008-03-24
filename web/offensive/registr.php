<?php
	set_include_path("..");
	require_once( 'offensive/assets/header.inc' );

	if( isset( $_SESSION['userid'] )) {
		header( "Location: ./" );
		exit;	
	}

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/validationFunctions.inc" );	
	require_once( "offensive/classes/statusMessage.inc" );

	$message;

	$username = trim( $_POST['username'] );
	$password = trim( $_POST['password'] );
	$referralcode = trim( $_POST['referralcode'] );
	
	$accountCreated = false;
	
	if( $_REQUEST['username'] != "" ) {
		$message = handleAccountRequest();
	}

	function handleAccountRequest() {
		
		global $username, $password, $referralcode, $accountCreated;
		
		$emailValidation = isValidEmail( $_REQUEST['email'] );
		
		if( ! $emailValidation->status ) {
			return $emailValidation;
		}
		
		if( isValidUsername( $username )
				&& isValidPassword( $password )
				&& $username != $password )
		{
			
			$result = createAccount( $username, $password, $referralcode );
			if( $result == "OK" ) {
				$message = new statusMessage( true, "Account created." );
				$accountCreated = true;
			} else {
				$message = new statusMessage( false, $result );
			}
			
		}
		else {
			$message = new statusMessage( false, "Invalid username or password. Passwords must be at least 5 characters long and may consist of letters, numbers, underscores, periods, and dashes. Passwords must not be the same as your username." );
		}
		
		return $message;
	
	}


	
	function getReferrerId( $refcode ) {

		$sql = "SELECT * FROM referrals WHERE referral_code = '$refcode' LIMIT 1";
		$result = tmbo_query( $sql );
		if( mysql_num_rows( $result ) == 1 ) {
			$row = mysql_fetch_assoc( $result );
			return $row['userid'];
		}

		return -1;

	}


	function createAccount( $uName, $pw, $referral ) {
	
		$returnMessage = "OK";
		
	    $link = openDbConnection();
	
		$referrerId = getReferrerId( $referral );
	
		if( $referrerId == -1 ) {
			return "Invalid referral code.";
		}

	    $query = "SELECT count(*) AS theCount FROM users WHERE username = '" . $uName . "'";

	    $result = tmbo_query($query);

		// get the results of the query as an associative array, indexed by column name
		$row = mysql_fetch_array( $result, MYSQL_ASSOC );
		
		if( $row['theCount'] == 0 ) {
			
            $encrypted_pw = sha1( $pw );
		
			$query = "INSERT INTO users (username,password,email,created,ip,referred_by) VALUES ( '" . $uName . "','" . $encrypted_pw . "', '" . $_POST['email'] . "', now(), '" . $_SERVER['REMOTE_ADDR']. "', $referrerId )";
			tmbo_query($query); 

			$result = tmbo_query("SELECT userid,account_status from users where username = '$uName'"); 
			$row = mysql_fetch_assoc( $result );
			if( $row['account_status'] == 'normal' ) {
				$_SESSION['userid'] = $row['userid'];
				$_SESSION['username'] = $uName;
			}
			
			$activationMessage = activationMessageFor( $row['userid'], $_POST['email'] );
			
			mail( $_POST['email'], "[ this might be offensive ] account activation", "$activationMessage", "From: offensive@thismight.be (this might be offensive)");
			
			/* this query not changed to tmbo_query
			 * because it should be non-fatal if the query fails. */
			mysql_query( "DELETE FROM referrals WHERE referral_code = '$referral' AND userid=$referrerId LIMIT 1" ) or trigger_error(mysql_error(), E_USER_WARNING);
#			mail( "ray@mysocalled.com", "[" . $_SERVER["REMOTE_ADDR"] . "] - [ this might be offensive ] account created: $uName", $_POST['email'], "From: offensive@thismight.be (this might be offensive)");
		} else {
		 	$returnMessage = "The username you've chosen, \"" . $uName . "\", is not available.";
		}
		
	    /* Closing connection */
	    mysql_close($link);
	    
    	return $returnMessage;
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 6.0.2">
	
	<link rel="stylesheet" type="text/css" href="/includes/style.css">
	<link rel="stylesheet" type="text/css" href="/styles/sparse.css"/>

	<script language="javascript">
		function setFocus() {
			if( document.forms[0] ) {
				document.forms[0].elements[0].focus();
			}
		}
		
		function checkPasswords() {
			if( document.forms["newaccount"].password.value != document.forms["newaccount"].password2.value ) {
				alert( "Password fields do not match." );
				document.forms["newaccount"].password.value = "";
				document.forms["newaccount"].password2.value = "";
				document.forms["newaccount"].password.focus();
				return false;
			} else {
				return true;
			}
		}
	</script>
	
	
	
</head>



<body>


<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
	<tr>
		<td valign="center" height="100%" align="center">

			<p>

			<?php if( ! $accountCreated ) { ?>
				<span class="small" >
					<div style="width:400px;margin-bottom:12px;"><?php echo $message->message?></div>
					<form name="newaccount" action="<? echo $_SERVER['PHP_SELF']?>" method="post" onSubmit="return checkPasswords()" method="post">
						<table>
							<tr>
								<td class="label">desired username:</td>
								<td><input type="text" name="username" size="20" value="<?php echo $username?>"/></td>
							</tr>
							<tr>
								<td class="label">password:</td>
								<td><input type="password" name="password" size="20"/></td>
							</tr>
							<tr>
								<td class="label">repeat password:</td>
								<td><input type="password" name="password2" size="20"/></td>
							</tr>
							<tr>
								<td class="label">email:</td>
								<td><input type="text" name="email" size="20" value="<?php echo $email?>"/></td>
							</tr>
							<tr>
								<td class="label">referral code:</td>
								<td><input type="text" name="referralcode" size="20" value="<?php echo $referralcode?>"/></td>
							</tr>
							<tr>
								<td colspan="2" class="submitcell">
									<input type="submit" class="button" value="register"/>
								</td>
							</tr>
						</table>
					</form>
				</span>
			<?php
				}
				else {
			?>
				<div class="normal">
					Thanks for registering!<br/>
					An email has been sent to you with instructions on how to activate your account.
				</div>

			<?php
				}
			?>

			</p>

		</td>
	</tr>
</table>
	
<? include '../includes/footer.txt' ?>

</body>


</html>