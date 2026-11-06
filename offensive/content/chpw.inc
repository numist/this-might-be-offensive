<?php
	// the password change form lives on the settings page now.  this keeps old
	// links and bookmarks working.
	function start() {
		mustLogIn();
		header("Location: ".Link::content("settings")."#password");
		exit;
	}
?>
