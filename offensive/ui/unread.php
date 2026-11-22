<?php
	/* Renders both unread comments lists (the sidebar and the nav popover) so
		offensive/js/unread.js can swap them in when a page comes back to the
		foreground or the realtime service says a subscription changed. */
	set_include_path("../..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	require_once("offensive/assets/unread.inc");

	mustLogIn();

	header("Content-Type: text/html; charset=utf-8");
	header("Cache-Control: no-store");

	unread();
	unread("unreadMenu-unread");
?>