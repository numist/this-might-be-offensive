<?php

// this code generates an XML file for all the markers and clusters shown
// on the map

set_include_path("../..");

require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");

// authentication
mustLogIn(array("prompt" => "http",
                "token" => null));

// filter users by these criteria
$filter = "AND u.account_status != 'locked' AND u.timestamp > DATE_SUB( NOW(), INTERVAL 12 MONTH )";

// this array determines how much of an area is mapped to a certain
// lat/lon at each zoom level. This is not an exact science, but may require
// some trial and error. keep each level a factor of the others.
$zoom_distance = array (
    0   =>  16,
    1   =>  16,
    2   =>  8,
    3   =>  4,
    4   =>  2,
    5   =>  1,
    6   =>  0.5,
);

$max_marker_level = 6;
$min_zoom = array();
$clusters = array();

// fill an array with $max_marker_level for each marker
$sort_markers = sql_get_markers($max_marker_level);

// loop over all levels, for each level check which markers overlap and which
// dont overlap. Markers that dont overlap get placed permanently from that
// level on. If they do overlap, place a group marker for that cluster
// there is a lot of waste here, but this should be fixed by changing the
// heavy work to insert/delete. It may be an idea to create a temp table
// thats a copy of the array that's created here. Invalidate temp table on
// changes. 
for($i = $max_marker_level; $i >= 0; $i--) {
    $markers = sql_get_markers_at_zoom($zoom_distance[$i]);

    foreach($markers as $index => $marker) {
        if($marker['num_users'] == 1) {
            $marker_id = $marker['userid'];
            $min_zoom[$marker_id] = $i;
        } else {
            $cluster = array(
                'lat'   =>  $marker['lat'],
                'lon'   =>  $marker['lon'],
                'level' =>  $i,				// level of this cluster
				'num_users' => $marker['num_users'],	// number of markers
				'swx' => $marker['swx'],
				'swy' => $marker['swy'],		
				'nex' => $marker['nex'],
				'ney' => $marker['ney']	
            ); 
            array_push($clusters,$cluster);
        }
    }
}

// now create the JSON file
header("Content-type: application/json");
$data = array(
	'markers'  => array(),
	'members'  => 0,
	'clusters' => array()
);

$num_users = 0;
// Iterate through the rows
foreach($sort_markers as $index => $marker) {
	$m = array(
		'user'  	=> htmlEscape($marker['username']),
		'userid' 	=> htmlEscape($marker['userid']), 
		'lat' 		=> $marker['x'],
		'lon' 		=> $marker['y'],
	);

    $id = $marker['userid'];

    if(isset($min_zoom[$id])) {
    	$m['minzoom'] = $min_zoom[$id];
    } else {
    	$m['minzoom'] = $max_marker_level+1;
    }
    $data['markers'][] = $m;
}
$data['members'] = sizeof($sort_markers);

// add all the cluster icons
foreach($clusters as $index => $cluster) {
	$c = array(
		'lat' => $cluster['lat'],
		'lon' => $cluster['lon'],
		'num_users' => $cluster['num_users'],
		'swx' => $cluster['swx'],
		'swy' => $cluster['swy'],
		'nex' => $cluster['nex'],
		'ney' => $cluster['ney'],
		'level' => $cluster['level']
	);
    $data['clusters'][] = $c;
}

echo json_encode($data);

function sql_get_markers($max_marker_level) {
	global $filter;
	$markers = array();

	$query = "SELECT map.userid, u.username,
	                 map.x, map.y
	          FROM maxxer_locations map, users u
						WHERE map.mapversion = 'google' AND map.userid = u.userid $filter
						ORDER BY u.username";

	$result = mysql_query($query);
	if (!$result) {
	    die('Invalid query: ' . mysql_error());
	}

	while ($row = @mysql_fetch_assoc($result)) {
	  	if(!me()->squelched($row["userid"])) {
		    $row['min_zoom'] = $max_marker_level+1;
		    array_push($markers, $row);
    	}
	}
	return($markers);
}


function sql_get_markers_at_zoom($level) {
	global $filter;

	$sql = "SELECT map.userid, u.username,
	               MIN(map.x) as swx, MIN(map.y) as swy,
	               MAX(map.x) as nex, MAX(map.y) as ney,
	               AVG(map.x) as lat, AVG(map.y) AS lon,
	               count(*) as num_users,
	               FLOOR(map.x/$level)*$level as fuzzy_lat, FLOOR(map.y/$level)*$level as fuzzy_lon
	        FROM maxxer_locations map, users u
	        WHERE map.mapversion = 'google' AND map.userid = u.userid $filter
	        GROUP BY fuzzy_lat, fuzzy_lon ORDER BY num_users DESC";

	$markers = array();
	$result = mysql_query($sql);
	if (!$result) {
	    die('Invalid query: ' . mysql_error());
	}
	while ($row = @mysql_fetch_array($result)) {
		if(!me()->squelched($row["userid"])) {
	    array_push($markers,$row);
		}
	}
	return($markers);
}


?>
