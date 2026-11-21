<?php

require_once("offensive/assets/core.inc");

// the top $n images of every year, as year => rank => Upload, newest year
// first. ranking a year means sorting all of its images, so the ranks are
// worked out in one pass over the whole table and kept in redis for an
// hour (hofYearRanks()). the rows themselves are read fresh, so counts are
// current and squelched uploaders are left out per viewer (leaving a gap
// in the ranks rather than renumbering the rest).
function hofYears($n) {
	$ranks = hofYearRanks($n);

	$byid = array();
	if(count($ranks) > 0) {
		$ids = array_map(function($r) { return $r[0]; }, $ranks);
		$result = tmbo_query(uploadsql()." AND up.id IN (".implode(",", $ids).")");
		while($row = mysqli_fetch_assoc($result)) {
			$upload = new Upload($row);
			if(!$upload->squelched()) {
				$byid[$upload->id()] = $upload;
			}
		}
	}

	$years = array();
	foreach($ranks as list($id, $year, $rank)) {
		if(!isset($years[$year])) {
			$years[$year] = array();
		}
		if(isset($byid[$id])) {
			$years[$year][$rank] = $byid[$id];
		}
	}
	return $years;
}

// the ranks behind hofYears(), as a list of array(id, year, rank) in the
// order the by year view lists them.
function hofYearRanks($n) {
	global $redis;
	$key = "hof:years:$n";
	$ranks = null;
	try {
		$cached = $redis ? $redis->get($key) : null;
		$ranks = $cached ? json_decode($cached, true) : null;
	} catch(Predis\PredisException $e) {
		// no redis: rank them every time
	}

	if(!is_array($ranks)) {
		// the same set and order as the per-year view (core_getuploads
		// with votes_desc): normal images with a user, ca.id breaking ties.
		// YEAR() and the per-year view's date bounds both use the
		// connection's time zone, so they agree on where a year ends.
		$result = tmbo_query("SELECT id, yr, rnk FROM (
				SELECT up.id, YEAR(up.timestamp) AS yr,
				       ROW_NUMBER() OVER (PARTITION BY YEAR(up.timestamp)
				                          ORDER BY ca.good DESC, ca.id DESC) AS rnk
				FROM offensive_uploads up
				JOIN offensive_count_cache ca ON ca.threadid = up.id
				JOIN users u ON u.userid = up.userid
				WHERE up.status = 'normal' AND up.type = 'image'
			) ranked
			WHERE rnk <= $n
			ORDER BY yr DESC, rnk ASC");
		$ranks = array();
		while($row = mysqli_fetch_assoc($result)) {
			$ranks[] = array((int)$row['id'], (int)$row['yr'], (int)$row['rnk']);
		}
		try {
			if($redis) $redis->setex($key, 60 * 60, json_encode($ranks));
		} catch(Predis\PredisException $e) {
			// next time, then
		}
	}
	return $ranks;
}

?>