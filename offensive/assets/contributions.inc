<?php

// a user's uploads, as listed on their profile (a preview) and on
// ?c=contributions (all of them, a page at a time): newest first with a
// heading per year, by votes, or by votes within each year.

// the sorts, as ?sort= value => label.
function contributionsSorts() {
	return array("" => "newest", "votes" => "votes, all time", "years" => "votes, by year");
}

// the sort asked for, or "" for newest first.
function contributionsSort() {
	return isset($_REQUEST['sort']) && is_string($_REQUEST['sort'])
	       && array_key_exists($_REQUEST['sort'], contributionsSorts()) ? $_REQUEST['sort'] : "";
}

function contributionsLink($page, $usrid, $sort) {
	return Link::content($page)."&userid=".(int)$usrid.($sort === "" ? "" : "&sort=$sort");
}

// the "sort by" line. on the profile the links jump down to the list.
function contributionsSortLinks($page, $usrid, $sort) {
	$anchor = $page == "user" ? "#contributions" : "";
?>
	<p>sort by:
<?php
	$first = true;
	foreach(contributionsSorts() as $key => $label) {
		echo $first ? "" : " · ";
		$first = false;
		if($key === $sort) {
			?><strong><?= $label ?></strong><?php
		} else {
			?><a href="<?= contributionsLink($page, $usrid, $key).$anchor ?>"><?= $label ?></a><?php
		}
	}
?>
	</p>
<?php
}

// a page of a user's uploads in the given sort. the votes sorts leave out
// discussions, which can't be voted on.
// a user's uploads come from the userid index, and even a prolific user
// has few enough that sorting them all is cheap. without the hint MariaDB
// intersects userid with the status index, which takes twice as long for
// a user with 20,000 uploads, and core_getuploads' newest first hint
// (t_t_id) needs a type, so without one it reads the whole table.
function contributionsQuery($usrid, $sort, $limit, $offset) {
	$usrid = (int)$usrid;
	$limit = (int)$limit;
	$offset = (int)$offset;
	if($sort == "votes") {
		$where = "AND up.type <> 'topic'";
		$order = "ca.good DESC, ca.id DESC";
	} else if($sort == "years") {
		$where = "AND up.type <> 'topic'";
		$order = "YEAR(up.timestamp) DESC, ca.good DESC, ca.id DESC";
	} else {
		$where = "";
		$order = "up.timestamp DESC, up.id DESC";
	}
	$sql = uploadsql()." AND up.userid = $usrid $where
	       ORDER BY $order
	       LIMIT $limit OFFSET $offset";
	$sql = str_replace("offensive_uploads up", "offensive_uploads up USE KEY (userid)", $sql);
	$result = tmbo_query($sql);
	$uploads = array();
	while($row = mysqli_fetch_assoc($result)) {
		$uploads[] = new Upload($row);
	}
	return $uploads;
}

// the user's top $n uploads of each year, newest year first, ranked like
// the hall of fame's by year view. the ranking reads only ids and counts;
// the full rows are fetched for the few uploads that make the cut.
function contributionsByYear($usrid, $n) {
	$usrid = (int)$usrid;
	$result = tmbo_query("SELECT id FROM (
			SELECT up.id, YEAR(up.timestamp) AS yr,
			       ROW_NUMBER() OVER (PARTITION BY YEAR(up.timestamp)
			                          ORDER BY ca.good DESC, ca.id DESC) AS rnk
			FROM offensive_uploads up USE KEY (userid)
			JOIN offensive_count_cache ca ON ca.threadid = up.id
			WHERE up.userid = $usrid AND up.status = 'normal' AND up.type <> 'topic'
		) ranked
		WHERE rnk <= $n
		ORDER BY yr DESC, rnk ASC");
	$ids = array();
	while($row = mysqli_fetch_assoc($result)) {
		$ids[] = (int)$row['id'];
	}
	if(count($ids) == 0) {
		return array();
	}
	$byid = array();
	$result = tmbo_query(uploadsql()." AND up.id IN (".implode(",", $ids).")");
	while($row = mysqli_fetch_assoc($result)) {
		$byid[$row['id']] = new Upload($row);
	}
	$uploads = array();
	foreach($ids as $id) {
		if(isset($byid[$id])) {
			$uploads[] = $byid[$id];
		}
	}
	return $uploads;
}

// a list of uploads. with $byYear, the uploads come newest year first and
// each year gets a heading and a list of its own, like the hall of fame's
// by year view.
function contributionsList($uploads, $byYear) {
	$year = null;
	foreach($uploads as $upload) {
		$thisYear = substr($upload->timestamp(), 0, 4);
		if($year === null || ($byYear && $thisYear !== $year)) {
			if($year !== null) {
				?></ul>
<?php
			}
			if($byYear) {
				?><h3><?= $thisYear ?></h3>
<?php
			}
			?>
	<ul class="imagelist">
<?php
			$year = $thisYear;
		}
		?>
		<li>
<?php
		// a discussion has no votes, so its bubble goes before the title
		// like the discussions list, and the counts column is left empty.
		// where the bubble sits tells a discussion from an image, so only
		// other types (audio) keep a type prefix.
		if($upload->type() == 'topic') {
?>
			<div><a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel(false) ?></a> <a href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename() ?></a></div>
<?php
		} else {
?>
			<div><a href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename($upload->type() != 'image') ?></a></div>
			<div><a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel() ?></a></div>
<?php
		}
?>
		</li>
		<?php
	}
	if($year !== null) {
		?></ul>
<?php
	}
}

?>