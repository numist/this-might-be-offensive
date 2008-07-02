<?php
	set_include_path("..");
	require_once('offensive/assets/header.inc');
	require_once('offensive/assets/logn.inc');

	/* there is always a redirect. 
	 * the user can specify the redirect in the _REQUEST['redirect'] variable.
	 * the location of the redirect defaults to /offensive/?c=main
	 */
	if(array_key_exists("redirect", $_REQUEST))
		$redirect = $_REQUEST['redirect'];
	else {
		$c = (array_key_exists("thumbnails", $_COOKIE) && 
		      $_COOKIE["thumbnails"] == "yes") ? 
		      "thumbs" : "main";
		$redirect = './?c='.$c;
	}

	// if the user is logged in already, redirect.
	if(loggedin()) {
		header( "Location: " . $redirect );
		exit;
	}

	$login_message = "";
	$prompt = true;

	$success = login($_REQUEST['username'], $_REQUEST['password']);
	if($success === true) {
		header( "Location: " . $redirect );
		exit;
	}
	
	// login attempt was ignored on purpose, and included a password
	if($success === null && array_key_exists('password', $_REQUEST)) {
		$prompt = false;
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">

<html>
<head>
	<title><?= $_SERVER['SERVER_NAME'] ?> : do we know you?</title>
	<link rel="stylesheet" type="text/css" href="/styles/sparse.css"/>
</head>


<body bgcolor="#ffffff">

	<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
		<tr>
			<td valign="center" height="100%" align="center">
	
				<p>

				<span class="small">
					<form action="./logn.php" method="post">

						<table>
							<tr>
								<td colspan="2"><p><?php echo $login_message ?></p></td>
							</tr>
							<? if($prompt) { ?>
							<tr>
								<td class="label"><p>user name:</p></td>
								<td><input type="text" name="username" size="12"/></td>
							</tr>
							<tr>
								<td class="label"><p>password:</p></td>
								<td><input type="password" name="password" size="12"/></td>
							</tr>
							<tr>
								<td class="label"></td>
								<td><p><input type="checkbox" name="rememberme" id="rememberme" value="1"><label for="rememberme">remember me</label></p></td>
							</tr>
							<tr>
								<td colspan="2" class="submitcell">
									<input type="submit" class="button" value="log in"/>
								</td>
							</tr>
							<? } ?>
						</table>
						<input type="hidden" name="redirect" value="<? echo array_key_exists("redirect", $_REQUEST) ? $_REQUEST['redirect'] : "./" ?>" />
					</form>
				
				</span>

				</p>
	
			</td>
		</tr>
	</table>
	
<? include 'includes/footer.txt' ?>

</body>
</html>
