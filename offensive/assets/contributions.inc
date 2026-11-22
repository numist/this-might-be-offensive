<?php

// a user's uploads, as listed on their profile (the newest few) and on
// ?c=contributions (all of them, a page at a time): newest first with a
// heading per year, by votes, or by votes within each year, as a list or,
// on ?c=contributions with &view=thumbs, as thumbnails.

// the sorts, as ?sort= value => label.
function contributionsSorts() {
	return array("" => "newest", "votes" => "votes, all time", "years" => "votes, by year");
}

// the sort asked for, or "" for newest first.
function contributionsSort() {
	return isset($_REQUEST['sort']) && is_string($_REQUEST['sort'])
	       && array_key_exists($_REQUEST['sort'], contributionsSorts()) ? $_REQUEST['sort'] : "";
}

// whether the thumbnail view was asked for.
function contributionsThumbs() {
	return isset($_REQUEST['view']) && $_REQUEST['view'] === "thumbs";
}

function contributionsLink($usrid, $sort, $thumbs = false) {
	return Link::content("contributions")."&userid=".(int)$usrid.($sort === "" ? "" : "&sort=$sort").($thumbs ? "&view=thumbs" : "");
}

// what a pic page link carries to browse this list (see picContext.inc).
function contributionsContextQuery($usrid, $sort, $thumbs = false) {
	return "&ctx=contributions&userid=".(int)$usrid.($sort === "" ? "" : "&sort=$sort").($thumbs ? "&view=thumbs" : "");
}

// the "sort by" and "view" lines.
function contributionsSortLinks($usrid, $sort, $thumbs) {
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
			?><a href="<?= contributionsLink($usrid, $key, $thumbs) ?>"><?= $label ?></a><?php
		}
	}
?>
	<br>view:
<?php
	foreach(array("list" => false, "thumbnails" => true) as $label => $view) {
		echo $view ? " · " : "";
		if($view == $thumbs) {
			?><strong><?= $label ?></strong><?php
		} else {
			?><a href="<?= contributionsLink($usrid, $sort, $view) ?>"><?= $label ?></a><?php
		}
	}
	if(!$thumbs) {
		?><br><?php commentPreviewsLink();
	}
?>
	</p>
<?php
}

// whether the viewer wants the start of a comment under each entry of a
// profile's recent remarks and contributions lists.
function commentPreviewsOn() {
	return me()->getPref("comment_previews") == 1;
}

function commentPreviewsLink() {
	if(commentPreviewsOn()) {
		?><a href="<?= Link::setPref("comment_previews", 0) ?>">hide previews</a><?php
	} else {
		?><a href="<?= Link::setPref("comment_previews", 1) ?>">show previews</a><?php
	}
}

function commentPreviewsToggle() {
	?><p><?php commentPreviewsLink(); ?></p>
<?php
}

// the start of a comment on one line, short enough to keep the page small.
// CSS trims it further to fit the width.
function commentPreview($text) {
	$text = trim(preg_replace('/\s+/u', ' ', mb_scrub($text, "UTF-8")));
	?><p><?= htmlEscape(mb_strimwidth($text, 0, 120, "…", "UTF-8")) ?></p><?php
}

// the uploader's first comment on each of their uploads, as fileid => text:
// the body of a discussion, or what they said about their image. one range
// of the (userid, fileid) index, in index order, so no sort.
function contributionsCaptions($usrid, $uploads) {
	$ids = array();
	foreach($uploads as $upload) {
		$ids[] = (int)$upload->id();
	}
	if(count($ids) == 0) {
		return array();
	}
	$sql = "SELECT fileid, comment FROM offensive_comments USE KEY (userid_fileid)
	        WHERE userid = ".(int)$usrid." AND fileid IN (".implode(",", $ids).")
	        AND comment <> ''
	        ORDER BY fileid, id";
	$result = tmbo_query($sql);
	$captions = array();
	while($row = mysqli_fetch_assoc($result)) {
		if(!array_key_exists($row['fileid'], $captions)) {
			$captions[$row['fileid']] = $row['comment'];
		}
	}
	return $captions;
}

// a page of a user's uploads in the given sort. the votes sorts leave out
// discussions, which can't be voted on, and so do thumbnails, since
// discussions have none.
// a user's uploads come from the userid index, and even a prolific user
// has few enough that sorting them all is cheap. without the hint MariaDB
// intersects userid with the status index, which takes twice as long for
// a user with 20,000 uploads, and core_getuploads' newest first hint
// (t_t_id) needs a type, so without one it reads the whole table.
function contributionsQuery($usrid, $sort, $limit, $offset, $thumbs = false) {
	$usrid = (int)$usrid;
	$limit = (int)$limit;
	$offset = (int)$offset;
	$where = $sort != "" || $thumbs ? "AND up.type <> 'topic'" : "";
	if($sort == "votes") {
		$order = "ca.good DESC, ca.id DESC";
	} else if($sort == "years") {
		$order = "YEAR(up.timestamp) DESC, ca.good DESC, ca.id DESC";
	} else {
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

// a list of uploads. with $byYear, the uploads come newest year first and
// each year gets a heading and a list of its own, like the hall of fame's
// by year view. $ctxQuery goes on each pic page link, so the pic page
// steps through this list. with $thumbs, the lists are of thumbnails like
// the thumbnail view of the image list (?c=thumbs), and the page puts them
// in a .grid_thumbs box. $captions (see contributionsCaptions()) puts the
// start of the uploader's comment under each title that has one.
function contributionsList($uploads, $byYear, $ctxQuery = "", $thumbs = false, $captions = null) {
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
	<ul<?= $thumbs ? "" : ' class="imagelist"' ?>>
<?php
			$year = $thisYear;
		}
		if($thumbs) {
			require("offensive/templates/thumbitem_file.inc");
			continue;
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
			<div><a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel(false) ?></a> <a href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename() ?></a><?php contributionsCaption($captions, $upload) ?></div>
<?php
		} else {
?>
			<div><a href="<?= Link::upload($upload).$ctxQuery ?>"><?= $upload->htmlFilename($upload->type() != 'image') ?></a><?php contributionsCaption($captions, $upload) ?></div>
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

function contributionsCaption($captions, $upload) {
	if(is_array($captions) && array_key_exists($upload->id(), $captions)) {
		commentPreview($captions[$upload->id()]);
	}
}

?>