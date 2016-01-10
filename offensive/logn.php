<?php
	set_include_path("..");
	require_once('offensive/assets/header.inc');
	require_once("offensive/assets/classes.inc");
	require_once("offensive/classes/assets.inc");

	/* the user can specify the redirect in the $_REQUEST['redirect'] variable.
	 * the location the redirect defaults to is /offensive/?c=main
	 */
	if(array_key_exists("redirect", $_REQUEST) && strlen($_REQUEST["redirect"]) > 0) {
		if(strpos($_REQUEST['redirect'], "//") === false) {
			$redirect = "https://".$_SERVER["HTTP_HOST"].$_REQUEST['redirect'];
		} else {
			$redirect = $_REQUEST['redirect'];
		}
	} else {
		$redirect = "";
	}

	// if the user is logged in already, redirect.
	if(login()) {
		/*
		 * if no redirect was requested, use the correct one from the
		 * user's preferences.
		 */
		if($redirect == "") {
			$redirect = Link::mainpage();
		}
		header( "Location: " . $redirect );
		exit;
	}

	$success = false;
	
	$name = isset($_REQUEST['howsername']) ? $_REQUEST['howsername'] : null;
	$pw = isset($_REQUEST['password']) ? $_REQUEST['password'] : null;
	if(login(array("u/p" => array($name, $pw)))) {
		/*
		 * if no redirect was requested, use the correct one from the
		 * user's preferences.
		 */
		if(!$redirect) {
			$redirect = Link::mainpage();
		}
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
	<title><?= $_SERVER['HTTP_HOST'] ?> : do we know you?</title>
	<link rel="stylesheet" type="text/css" href="/styles/sparse.css"/>
	<?
		include_once("analytics.inc");
		JS::add("/offensive/js/jquery-1.7.1.min.js");
		JS::emit();
	?>

</head>


<body bgcolor="#ffffff" onLoad="document.forms[0].howsername.focus();">

	<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
		<tr>
			<td valign="center" height="100%" align="center">
	
				<p>

				<span class="small">
					<form action="./logn.php" method="post">
						<!-- <?= sha1("tester") ?> -->
						<table>
							<tr>
								<td colspan="2"><p><?php echo $login_message ?></p></td>
							</tr>
							<? if($prompt) { ?>
							<tr>
								<td class="label"><p>user name:</p></td>
								<td><input type="text" name="howsername" size="12"/></td>
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
						<? if (isOpenRegistration()) { ?>
							<p id="signup_link" style="text-align: center; opacity: 0;">
								new here? it's your lucky day.<br />
								<a href="/offensive/registr.php">create an account</a>
							</p>
						<? } ?>
						<? if($redirect) { ?>
							<input type="hidden" name="redirect" value="<?= $redirect ?>" />
						<? } ?>
					</form>
				
				</span>

				</p>
	
			</td>
		</tr>
	</table>

	
	
<? include 'includes/footer.txt' ?>

<? 
   // If the non-referred registration of the day hasn't been
   // taken yet, fade the registration link in after 3 seconds
   // of non-activity.

   if (isOpenRegistration()) { ?>
	<script>
		var timeoutId = -1;
		resetLinkTimeout();

		$('html').keydown(function(e) {
			resetLinkTimeout();
		});

		function resetLinkTimeout() {
			if (timeoutId != -1)
				clearTimeout(timeoutId);

			timeoutId = setTimeout(function() {
				$("#signup_link").animate({opacity: 1}, 'slow');
			}, 3000);
		}
	</script>
<? } ?>

</body>
</html>
