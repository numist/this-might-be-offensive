#!/usr/bin/php
<?php

// import images from TMBO into imgSeek
// run from documentroot  'admin/isk/isk_import.php STARTID ENDID' to import the images whose
// upload ids are in that range (inclusive)

if (php_sapi_name() != "cli") {
	trigger_error("this script can only be run from the command line", E_USER_ERROR);
	exit(1);
}

if ($argc != 3 || !ctype_digit($argv[1]) || !ctype_digit($argv[2])) {
	fwrite(STDERR, "usage: php {$argv[0]} STARTID ENDID\n");
	exit(1);
}

if(!($config = parse_ini_file("admin/.config"))) {
        trigger_error("No configuration file found", E_USER_ERROR);
        exit;
}

// isk class
require_once("offensive/assets/isk.inc");

// need xmlrpc client lib
require_once("offensive/assets/xmlrpc.inc");

// fetch all images from the DB
$images = fetch_db_images($config['database_user'], 
                          $config['database_pass'], 
                          $config['database_name'], 
                          $config['database_host']);

// add an image path to all images
$images = add_image_path($images);

// add images to ISK
add_to_isk($images);
echo "Done.\n";


// add the path to the images array, we need that to tell ISK where the image is
function add_image_path($images = array()) {
	$new_images = array();
	foreach($images as $image) {
		$path = imgPath($image['timestamp'], $image['id'], $image['filename']);
		if(!file_exists($path)) continue;
		$image['path'] = $path;
		$new_images[] = $image;
	}
	return $new_images;
}

// calculate path based on timestamp
function imgPath($timestamp, $id, $filename) {
	if(!is_intger($timestamp, $id, $filename))
		$timestamp = strtotime($timestamp);
	
	$year = date( "Y", $timestamp );
	$month = date( "m", $timestamp );
	$day = date( "d", $timestamp );
	
	$path = "offensive/uploads/$year/$month/$day/image/" . $id . "_" . $filename;
	
	return($path);
}

// grab all images from the DB
function fetch_db_images($dbuser = "", $dbpass = "", $database= "", $dbhost = "") {
  
	// connect to database
	mysqli_report(MYSQLI_REPORT_OFF);
	$db = mysqli_connect($dbhost, $dbuser, $dbpass);
	if (!$db) {
	    die('Could not connect: ' . mysqli_connect_error());
	}

	// the tables are latin1 but hold UTF-8 bytes; talking latin1 passes them through
	// untouched, so filenames match what's on disk
	if (!mysqli_set_charset($db, "latin1")) {
	    die('Could not set charset: ' . mysqli_error($db));
	}

	// select DB
	$db_selected = mysqli_select_db($db, $database);
	if (!$db_selected) {
	    die ("Can't use $database : " . mysqli_error($db));
	}

	// read images from database (both ids were checked to be digits on startup)
	$startid = (int)$_SERVER['argv'][1];
	$endid = (int)$_SERVER['argv'][2];

	$sql = "SELECT * FROM offensive_uploads WHERE type='image' AND status='normal'
	                                        AND id >= $startid AND id <= $endid
	                                        ORDER by timestamp ASC";

	$result = mysqli_query($db, $sql);
	if (!$result) {
	    die('Could not fetch data: ' . mysqli_error($db));
	}
	$rows = array();
	while($row = mysqli_fetch_assoc($result)) {
		$rows[] = $row;
	}
	mysqli_close($db);
	return $rows;
}

function add_to_isk($images) {

	// create ISK object
	$isk = new Isk();

	// loop over images and add them to ISK
	$counter = 0;
	foreach($images as $image) {
		$counter++;
		if(!($counter % 500)) {
		  $isk->save();
		}
		if(!$isk->add($image["path"], $image['id'])) {
		  if($isk->error()) {
		    echo "failed " . $isk->error_msg();
		  } else {
		    echo "skipping " . $image['path'];
		  }
		} else {
		  echo "adding " . $image['path'];
		}
		echo "\n";
	}
	$isk->save();
	unset($isk);
}

// ok, just ripped this from TMBO
function is_intger($arg) {
        return (is_numeric($arg) && floor($arg) == ceil($arg));
}

