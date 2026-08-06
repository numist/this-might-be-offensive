<?php
	// must match header.inc; see the note there.  this file includes nothing,
	// so the name is repeated rather than shared.
	session_name("TMBOSESS1");
	session_start();
	session_unset();
	// flags must match the ones issueRememberCookie() sets in logn.inc.
	setcookie( "remember", false, time()-4200, "/", "", true, true );
	// redirect to the main page
	header("Location: ./logn.php");
?>