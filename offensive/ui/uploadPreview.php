<?php
	/* Sends the resize tool's preview (offensive/assets/upload.inc) to the
		person whose upload it is. The preview lives in the quarantine, which
		isn't served, and its name comes from this session, so nobody else can
		reach it. */
	set_include_path("../..");
	require_once("offensive/assets/header.inc");

	mustLogIn();

	$postdata = array_key_exists("postdata", $_SESSION) ? unserialize($_SESSION['postdata']) : false;
	$preview = is_array($postdata) && array_key_exists("preview", $postdata) ? $postdata['preview'] : false;
	$file = $preview ? "../quarantine/".basename($preview['tmpname']) : "";

	if(!$preview || $preview['userid'] != me()->id() || !is_file($file)) {
		header("HTTP/1.0 404 Not Found");
		header("Cache-Control: no-store");
		exit;
	}

	$types = array(IMAGETYPE_JPEG => "image/jpeg", IMAGETYPE_PNG => "image/png",
	               IMAGETYPE_GIF => "image/gif", IMAGETYPE_WEBP => "image/webp");
	$info = getimagesize($file);
	if(!$info || !array_key_exists($info[2], $types)) {
		header("HTTP/1.0 404 Not Found");
		header("Cache-Control: no-store");
		exit;
	}

	while(ob_get_level() > 0) ob_end_clean();
	header("Content-Type: ".$types[$info[2]]);
	header("Content-Length: ".filesize($file));
	header("Cache-Control: private, no-store");
	readfile($file);
?>
