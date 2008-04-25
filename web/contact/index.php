<?php
set_include_path("..");
require_once("offensive/assets/header.inc");

// Attempt to defend against header injections:
$badStrings = array("Content-Type:",
                     "MIME-Version:",
                     "Content-Transfer-Encoding:",
                     "bcc:",
                     "cc:",
                     "multipart/mixed",
                     "charleslegbe@aol.com",
                     "hometown.aol.com");

// Loop through each POST'ed value and test if it contains
// one of the $badStrings:
foreach($_POST as $k => $v){
	foreach($badStrings as $v2){
		if(strpos($v, $v2) !== false){
			logBadRequest();
			header("HTTP/1.0 403 Forbidden");
			exit;
		}
	}
}  

function logBadRequest() {
	mail( "ray@mysocalled.com",
			 "[" . $_SERVER["REMOTE_ADDR"] . "] - contact form abuse",
			 "[" . $_SERVER["REMOTE_ADDR"] . "] - contact form abuse\n\n" . requestDetail(),
			"From: themaxx.com contact form" );	
}


	if( isset($body) && $body != "" ) {

		if(ini_get("magic_quotes_gpc")) {
			// replace \' with '
			$body = str_replace( "\\'", "'", $body );
	
			// replace \" with "
			$body = str_replace( "\\\"", "\"", $body );
		}

		// email a copy of the new post to myself.
		mail( "ray@mysocalled.com", "$head", "$body\n\n--\n\n[$REMOTE_ADDR] - $from\n\n$email\n", "From: $email ($from via themaxx.com contact form)\nReply-To: $email");
	}
	
	
	function requestDetail() {
		ob_start();
		var_dump( $_SERVER );
		var_dump( $_REQUEST );		
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}

		
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title>freakin' contact form.</title>
	<link rel="stylesheet" type="text/css" href="/includes/style.css">
	
</head>


<body bgcolor="#333366" link="#000066" vlink="#000033">

<div align="center">

<?php require( "includes/headerbuttons.txt" ); ?>

<table border="0" cellpadding="0" cellspacing="12">
	<tr>
		<td valign="top">
						
			<table border="0" cellpadding="0" cellspacing="0">
				<tr>
					<td>
						
						<table border="0" cellpadding="0" cellspacing="0" width="100%" height="37">
							<tr>
								<td height="37">&nbsp;</td>
							</tr>
						</table>


				<tr>
					<td valign="top">
						<table border="0" cellpadding="2" cellspacing="0">
							<tr>
								<td bgcolor="#000000">
									
									<table border="0" cellpadding="0" cellspacing="0" width="171">
										<tr>
											<td><img src="/graphics/hbar.gif" width="171" height="11" alt=""></td>
										</tr>

										<tr>
											<td bgcolor="#ccccff">

												<table border="0" cellpadding="8" cellspacing="0" width="100%">
													<tr>
														<td align="left">
															<div class="small">
																<!--
																it has become clear to me that i cannot have my email address scattered throughout the site without being subjected to a mailbox full of spam every day, so i've constructed this wangdango of a message catapult. put your message in, pull the lever, and blammo! a new dent in my mailbox.
																<P>by the way, my email address is exactly what you would expect it to be, with my name being <b>ray</b> and the domain being <b>themaxx.com</b> and all, so if wielding the massive power of this machine isn't comfortable for you, or if you think you might accidentally destroy the universe with it, do the math and you can send it to me directly.
																-->
In a last ditch effort to preserve my remaining 2
splinters of sanity in the hopes that I might one
day rub them together and kindle a new inferno, I
have sacrificed my email address to the spam
gods once and for all. They may now have their way
with it (which they were doing anyway), and send
it all of the get rich quick while losing weight
fast and having sex with new low teen mortgage
rates with free herbal viagra schemes they can
come up with. They can even pay off my credit card
debt if they want to.

<P>My former email address is now officially a spam
graveyard.

<P>Fill in the form and pull the lever, and your message
will be stapled to a pigeon and flown directly to me.

															</div>
														</td>
													</tr>
												</table>

											</td>
										</tr>
										<tr>
											<td><img src="/graphics/hbar.gif" width="171" height="11" alt=""></td>
										</tr>
									</table>

								</td>
							</tr>			
						</table>
					</td>
				</tr>

				<tr><td height="12"></td></tr>
	
			</table>

		</td>


		<!-- right column -->
		<td rowspan=8 valign="top">

			<table border="0" cellpadding="0" cellspacing="0" width="100%">
				<tr>
					<td align="right"><img src="graphics/contact.gif" alt="contact" width="247" height="36" border="0"></td>
				</tr>
			</table>

			<table border="0" cellpadding="2" cellspacing="0">
				<tr>
					<td bgcolor="#000000">
						<table border="0" cellpadding="0" cellspacing="0" width="346">
							<tr>
								<td bgcolor="#ccccff"><img src="/graphics/maincolumntopbar.gif" width="346" height="12"></td>
							</tr>
							<tr>
								<td bgcolor="#ccccff">

									<table border="0" cellpadding="8" cellspacing="0">
										<tr>
											<td>
											
										<?php
											if( !isset($body) || $body == "" ) {
										?>
												<form action="./" method="post">
													<table border="0">
														<tr>
															<td align="right"><div class="small"><b>from:</b></div></td>
															<td><input type="text" name="from" size="30"></td>
														</tr>
														<tr>
															<td align="right"><div class="small"><b>your email:</b></div></td>
															<td><input type="text" name="email" size="30"></td>
														</tr>
														<tr>
															<td align="right"><div class="small"><b>subject:</b></div></td>
															<td><input type="text" name="head" size="30"></td>
														</tr>
														<tr>
															<td align="right" valign="top"><div class="small"><b>what say you:</b></div></td>
															<td><textarea name="body" rows="20" cols="30"></textarea></td>
														</tr>
														<tr><td align="center" colspan="2"><input type="submit" value="set this pigeon free!">
													</table>
												</form>
										<?php
											} else {
										?>
										
											<div class="normal">
												<P>bombs away!<br><br>
												your message has been sent.<br><br>
												thanks for getting in touch.
											</div>
										
										<?php
											}
										?>
											
											</td>
										</tr>
									</table>

								</td>
							</tr>
							<tr>
								<td bgcolor="#ccccff"><img src="/graphics/maincolumntopbar.gif" width="346" height="12"></td>
							</tr>
						</table>

					</td>
				</tr>
			</table>

		</td>
		<!-- end main column -->
		
	</tr>


	
	
</table>

</div>

<BR>


<div align="center">
	<?php require( "includes/textlinks.txt" ); ?>
</div>

<br>
</body>
</html>

