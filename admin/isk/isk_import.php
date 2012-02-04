#!/usr/bin/php
<?php

// import images from TMBO into imgSeek
// run from documentroot  'admin/isk/isk_import.php XX' where XX is number of months to import, default=1

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
	foreach($images as $image) {
		$path = imgPath($image['timestamp'], $image['id'], $image['filename']);
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
	$db = mysql_connect($dbhost, $dbuser, $dbpass);
	if (!$db) {
	    die('Could not connect: ' . mysql_error());
	}

	// select DB
	$db_selected = mysql_select_db($database, $db);
	if (!$db_selected) {
	    die ("Can't use $database : " . mysql_error());
	}

	// read images from database
	$months = ($_SERVER['argc'] > 1 && is_numeric($_SERVER['argv'][1]) ? $_SERVER['argv'][1] : 1);
	
	$sql = "SELECT * FROM offensive_uploads WHERE type='image' 
	                                        AND timestamp > DATE_SUB(NOW(), INTERVAL $months MONTH) 
	                                        ORDER by timestamp ASC";
	                                        
	$result = mysql_query($sql);
	if (!$result) {
	    die('Could not fetch data: ' . mysql_error());
	}
	while($row = mysql_fetch_assoc($result)) {
		$rows[] = $row;
	}
	mysql_close();
	return $rows;
}

function add_to_isk($images) {
	
	// create ISK object
	$isk = new Isk;

	// loop over images and add them to ISK
	$counter = 0;
	foreach($images as $image) {
	  if($isk->img_exists($image['id'])) {
	    echo "skipping " . $image['path'] . "\n";
	    continue;
	  } else {
		  echo "adding " . $image['path'] . "\n";
		}
		$counter++;
		if(!($counter % 500)) {
		  $isk->save();
		}
		$img = $isk->img_blob($image['path']);
		$isk->add($image['id'], $img);
	}
	$isk->save();
	unset($isk);
}

// ok, just ripped this from TMBO
function is_intger($arg) {
        return (is_numeric($arg) && floor($arg) == ceil($arg));
}

