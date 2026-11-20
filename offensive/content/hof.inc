<?php

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once('offensive/assets/tabs.inc');
	require_once('offensive/assets/functions.inc');
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
	}

	function head() {
		CSS::add("/styles/grid.css");
		JS::add("/offensive/js/lazyload_bottom.js");
	}
	// a year to show on its own, from ?y=, or false for the other views.
	function hofYear() {
		if(!isset($_REQUEST['y']) || !is_intger($_REQUEST['y'])) {
			return false;
		}
		$year = (int)$_REQUEST['y'];
		return $year >= 1997 && $year <= (int)date("Y") ? $year : false;
	}

	// the top $n images of every year, as year => rank => Upload, newest year
	// first. ranking a year means sorting all of its images, so the ranks are
	// worked out in one pass over the whole table and kept in redis for an
	// hour. the rows themselves are read fresh, so counts are current and
	// squelched uploaders are left out per viewer (leaving a gap in the ranks
	// rather than renumbering the rest).
	function hofYears($n) {
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

	// a ranked list of uploads, given as rank => Upload in rank order.
	function hofList($ranked, $start) {
		// the rows are grid boxes, not list items, so browsers don't apply
		// start and value to them; the inline counter styles do that.
?>
				<ol start="<?= $start ?>" style="counter-reset: list-item <?= $start - 1 ?>">
<?php
		$next = $start;
		foreach($ranked as $rank => $upload) {
			// cellophane doesn't want her images in the hof;
			// this is a quick hack to omit them.
			if( $upload->uploader()->id() == 1138 ) {
				continue;
			}
			// a row after a gap says its rank, so the list doesn't renumber.
			$value = $rank != $next ? ' value="'.$rank.'" style="counter-set: list-item '.$rank.'"' : "";
			$next = $rank + 1;
?>
					<li data-fileid="<?= $upload->id() ?>"<?= $value ?>>
						<div><a href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename() ?></a></div>
						<div class="score"><a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel(false) ?><span class="goods"><?= $upload->goods() ?></span></a></div>
						<div>
							<?= htmlEscape($upload->uploader()->username()) ?>
							<time datetime="<?= $upload->timestamp() ?>"><?= time_ago($upload->timestamp()) ?> ago</time>
						</div>
					</li>
<?php
		}
?>
				</ol>
<?php
	}

	function body() {

		$year = hofYear();
		$view = $year ? 'years' : (isset( $_REQUEST['t'] ) ? sqlEscape( $_REQUEST['t'] ) : 'hof');
		$what = $year ? "of $year" : ($view == 'years' ? "of each year" : "of all time");
?>
<div class="contentbox">
	<div class="heading">hall of fame: the coolest files <?= $what ?> (according to you).</div>
	<?php tabs(); ?>

	<div class="tabs subtabs">
		<div class="<?= ($view == 'hof' ? 'tabon' : 'taboff') ?>"><a href="<?= Link::content("hof") ?>">all time</a></div>
		<div class="<?= ($view == 'today' ? 'tabon' : 'taboff') ?>"><a href="<?= Link::content("hof") ?>&t=today">past 24 hours</a></div>
		<div class="<?= ($view == 'years' ? 'tabon' : 'taboff') ?>"><a href="<?= Link::content("hof") ?>&t=years">by year</a></div>
	</div>


	<div class="bluebox">
		<div id="grid-container">
			<div class="grid" id="hof">
<?php
		if($view == 'years' && !$year) {
			foreach(hofYears(10) as $y => $ranked) {
?>
				<h3><a href="<?= Link::content("hof") ?>&y=<?= $y ?>"><?= $y ?></a></h3>
<?php
				hofList($ranked, 1);
			}
?>
			</div>
		</div>
	</div>
</div>
<?php
			return;
		}

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$args = $_REQUEST;

		if(!array_key_exists("sort", $args)) {
			$args["sort"] = "votes_desc";
		}
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = "100";
		}
		if($view == "today") {
			$args["after"] = time() - (60 * 60 * 24);
		}
		if($year) {
			// strings, not unix times: they're compared in the connection's
			// time zone, the same one YEAR() uses on the by-year view.
			$args["after"] = ($year - 1)."-12-31 23:59:59";
			$args["before"] = ($year + 1)."-01-01 00:00:00";
		}
		if(!array_key_exists("type", $args)) {
			$args["type"] = "image";
		}

		// the list numbers the rows, starting from this page's first rank.
		$first = $page * $numPerPage + 1;
		$ranked = array();
		foreach(core_getuploads($args) as $i => $upload) {
			$ranked[$first + $i] = $upload;
		}
		hofList($ranked, $first);
?>
			</div>
				<div id="bottom">
					<?php
						// next page!
						$args = $_GET;
						if(array_key_exists("p", $args)) {
							$args["p"]++;
						} else {
							$args["p"] = 1;
						}
					?>
					<p id="morelink"><a href="<?= $_SERVER['SCRIPT_NAME'] ?>?<?= http_build_query($args) ?>">more…</a></p>
				</div>
			</div>
	</div>
</div>
<?php
	}
?>
