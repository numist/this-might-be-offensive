<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc' );

	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/activationFunctions.inc' );
	require_once( "offensive/assets/validationFunctions.inc" );
	require_once( 'offensive/assets/functions.inc' );
	
	if( isset($_REQUEST['x2']) ) {
		$code = $_REQUEST['x2'];
		$pw = $_REQUEST['password'];
		
		if( ($row = userRowFromCode( $_REQUEST['x2'] )) && isValidPassword( $pw ) && $pw == $_REQUEST['password2'] ) {
			
			$uid = $row['userid'];

            $encrypted_pw = sha1( $pw );

			$sql = "UPDATE users SET timestamp = timestamp, password='$encrypted_pw' WHERE userid = $uid LIMIT 1";
			
			tmbo_query( $sql );
			header( "Location: ./logn.php" );
		}	
		else {
			echo "There was a problem with your request. (Possibly unacceptable password or the entries didn't match.)";
		}
	}

	function sendResetEmail( $username ) {
		
		$username = sqlEscape( $username );
		$sql = "SELECT * FROM users WHERE username='$username'";
		$result = tmbo_query( $sql );
		if( mysql_num_rows( $result ) == 1 ) {
			$row = mysql_fetch_assoc( $result );
			$code = hashFromUserRow( $row );
			$message = "Someone (hopefully you) wants to reset your [this might be offensive] password. To reset your password, please visit the following link:

https://".$_SERVER['SERVER_NAME']."/offensive/pwreset.php?x=$code

			";
			
			if( isValidEmail( $row['email'] ) ) {

				mail( $row['email'], "resetting your [this might be offensive] password", $message, "From: offensive@thismight.be (this might be offensive)\r\n"/*bcc:ray@mysocalled.com"*/) or trigger_error("could not send email", E_USER_ERROR);

				echo "An email has been sent containing instructions for resetting your password.";
			}
			else {
				echo "Unfortunately, we don't have a valid email address for that account. There's nothing we can do for you.";
			}

		}

	}


	function hashFromUserRow( $row ) {
		$id = $row[ 'userid' ];
		$input = $row['username'] . $row['password'] . ":wakka";
		$code = tmbohash( $id, $input );
		return $code;
	}

	function emitResetForm( $code ) {
	?>
		<span class="small">
			<form action="<?= $_SERVER['PHP_SELF']?>" method="post">
				<input type="hidden" name="x2" value="<?= $code ?>"/>
				<input type="hidden" name="x" value="<?= $code ?>"/>
				<table>
					<tr>
						<td colspan="2"><?php echo $login_message ?></td>
					</tr>
					<tr>
						<td class="label">new password:</td>
						<td><input type="password" name="password" size="12"/></td>
					</tr>
					<tr>
						<td class="label">again:</td>
						<td><input type="password" name="password2" size="12"/></td>
					</tr>					
					<tr>
						<td colspan="2" class="submitcell">
							<input type="submit" class="button" value="do it."/>
						</td>
					</tr>
				</table>
			</form>
		
		</span>
		<?
	}

	
	function sendMessageForm() {
	?>
		<span class="small">
			<form action="<?= $_SERVER['PHP_SELF']?>" method="post">
				<table>
					<tr>
						<td colspan="2"><?php echo isset($login_message) ? $login_message : ""; ?></td>
					</tr>
					<tr>
						<td class="label">user name:</td>
						<td><input type="text" name="username" size="12"/></td>
					</tr>
					<tr>
						<td colspan="2" class="submitcell">
							<input type="submit" class="button" value="send me instructions for resetting my password"/>
						</td>
					</tr>
				</table>
			</form>
		
		</span>
		<?
	}
	
	function userRowFromCode( $code ) {
	
		$id = id_from_hash( $code );
		if( is_intger( $id ) && $id > 1 ) {
			$sql = "SELECT * FROM users WHERE userid = $id";
			$result = tmbo_query( $sql );
			if( mysql_num_rows( $result ) == 1 ) {
				$row = mysql_fetch_assoc( $result );
				$hash = hashFromUserRow( $row );
				if( $hash == $code ) {
					return $row;
				}
			}
		}
		return false;
	}
	

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">

<html>
<head>
	<title>tmbo.org : do we know you?</title>
	<link rel="stylesheet" type="text/css" href="/styles/sparse.css"/>
</head>


<body bgcolor="#ffffff">

	<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
		<tr>
			<td valign="center" height="100%" align="center">
	
				<p>

					<? 
						
						if( isset($_REQUEST['username']) ) {
							sendResetEmail( $_REQUEST['username'] );
						}
						else if( isset( $_REQUEST['x'] ) && userRowFromCode( $_REQUEST['x'] ) ) {
							emitResetForm( $_REQUEST['x'] );
						} else {
							sendMessageForm( );
						}
					?>
					
				</p>
	
			</td>
		</tr>
	</table>
	
<? include '../includes/footer.txt' ?>

</body>
</html>
