<?php

set_include_path("../..");
require_once("offensive/assets/header.inc");
require_once("offensive/assets/functions.inc");
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/classes.inc");
require_once("offensive/assets/core.inc");
require_once("offensive/assets/id3.inc");

function fail() {
	header("Location: http://{$_SERVER['HTTP_HOST']}/offensive/404.php", true, 301);
	exit;
}

$id = "";
if(array_key_exists("id", $_REQUEST)) {
	$id = $_REQUEST["id"];
}

if(!is_intger($id)) fail();

$upload = core_getupload($id);
if($upload->type() != "audio" || !$upload->file()) fail();

$fp = fopen($upload->file(), 'r');
$id3 = new getid3_id3v2($fp, $info);

// check for a valid id3 tag
if(!array_key_exists('id3v2', $info)) fail();

$artdata = false;

// different kinds of embeddable images:
if(array_key_exists('APIC', $info['id3v2'])
   && count($info['id3v2']['APIC']) > 0
   && array_key_exists('data', $info['id3v2']['APIC'][0])) {
	$artdata = $info['id3v2']['APIC'][0]['data'];
} else if(array_key_exists('PIC', $info['id3v2'])
          && count($info['id3v2']['PIC']) > 0
          && array_key_exists('data', $info['id3v2']['PIC'][0])) {
	$artdata = $info['id3v2']['PIC'][0]['data'];
}

if(!$artdata) fail();

// The tag's own MIME field is free text ("jpg", "PNG", "image/jpg", or empty)
// and nginx sends nosniff, so browsers won't guess past a bad one. Name the
// type from the bytes instead, and only for formats we'd accept as an upload.
$size = @getimagesizefromstring($artdata);
$allowed = [IMAGETYPE_JPEG, IMAGETYPE_PNG, IMAGETYPE_GIF, IMAGETYPE_WEBP];
if(!$size || !in_array($size[2], $allowed)) fail();

header("Content-Type: ".image_type_to_mime_type($size[2]));

echo $artdata;


?>