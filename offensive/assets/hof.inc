<?php

require_once("offensive/assets/core.inc");

// the top $n images of every year, as year => rank => Upload, newest year
// first. ranking a year means sorting all of its images, so the ranks are
// worked out by hofYearRanks() and kept in redis. the rows themselves are
// read fresh, so counts are current and squelched uploaders are left out
// per viewer (leaving a gap in the ranks rather than renumbering the rest).
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
// order the by year view lists them. this year's ranks still move, so they
// are kept for an hour and only cost a scan of this year's images. earlier
// years barely change but take a pass over the whole table (several seconds
// on production), so they are kept for a week, under a key that names this
// year so that a new year starts a new list.
function hofYearRanks($n) {
	// the database's year, not PHP's: YEAR() and the per-year view's date
	// bounds use the connection's time zone, so this agrees on where a
	// year ends.
	$row = mysqli_fetch_row(tmbo_query("SELECT YEAR(NOW())"));
	$year = (int)$row[0];

	return array_merge(
		hofYearRanksCached("hof:years:$n:$year", 60 * 60,
		                   "up.timestamp >= '$year-01-01'", $n),
		hofYearRanksCached("hof:years:$n:before:$year", 7 * 24 * 60 * 60,
		                   "up.timestamp < '$year-01-01'", $n));
}

// the ranks for the images matching $where, from redis if they're there,
// otherwise worked out and kept for $ttl seconds.
function hofYearRanksCached($key, $ttl, $where, $n) {
	global $redis;
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
		$result = tmbo_query("SELECT id, yr, rnk FROM (
				SELECT up.id, YEAR(up.timestamp) AS yr,
				       ROW_NUMBER() OVER (PARTITION BY YEAR(up.timestamp)
				                          ORDER BY ca.good DESC, ca.id DESC) AS rnk
				FROM offensive_uploads up
				JOIN offensive_count_cache ca ON ca.threadid = up.id
				JOIN users u ON u.userid = up.userid
				WHERE up.status = 'normal' AND up.type = 'image' AND $where
			) ranked
			WHERE rnk <= $n
			ORDER BY yr DESC, rnk ASC");
		$ranks = array();
		while($row = mysqli_fetch_assoc($result)) {
			$ranks[] = array((int)$row['id'], (int)$row['yr'], (int)$row['rnk']);
		}
		try {
			if($redis) $redis->setex($key, $ttl, json_encode($ranks));
		} catch(Predis\PredisException $e) {
			// next time, then
		}
	}
	return $ranks;
}

?>