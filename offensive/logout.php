<?php
	session_start();
	session_unset();
	setcookie( "remember", false, time()-4200, "/" );
	// redirect to the main page
	header("Location: ./logn.php");
?>