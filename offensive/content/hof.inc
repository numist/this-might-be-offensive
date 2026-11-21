<?php

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once('offensive/assets/tabs.inc');
	require_once('offensive/assets/functions.inc');
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	require_once("offensive/assets/hof.inc");
	require_once("offensive/assets/picContext.inc");

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

	// the first year with any images, so the per-year view knows where to stop.
	// t_t_id starts with type and timestamp, so this reads one index entry.
	function hofFirstYear() {
		$result = tmbo_query("SELECT YEAR(MIN(timestamp)) AS yr FROM offensive_uploads WHERE type = 'image'");
		$row = mysqli_fetch_assoc($result);
		return $row && $row['yr'] ? (int)$row['yr'] : (int)date("Y");
	}

	// a ranked list of uploads, given as rank => Upload in rank order.
	// $ctxQuery goes on each pic page link, so the pic page steps
	// through this list.
	function hofList($ranked, $start, $ctxQuery) {
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
						<div><a href="<?= Link::upload($upload).$ctxQuery ?>"><?= $upload->htmlFilename() ?></a></div>
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
				hofList($ranked, 1, hofContextQuery("years", false));
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

		if($year) {
			// newer year on the left, older on the right, like the by-year list
			// reads top to bottom. an empty span holds the side with no year.
?>
				<nav>
					<?= $year < (int)date("Y") ? '<a href="'.Link::content("hof").'&y='.($year + 1).'" rel="next">« '.($year + 1).'</a>' : '<span></span>' ?>
					<h3><?= $year ?></h3>
					<?= $year > hofFirstYear() ? '<a href="'.Link::content("hof").'&y='.($year - 1).'" rel="prev">'.($year - 1).' »</a>' : '<span></span>' ?>
				</nav>
<?php
		}

		// the list numbers the rows, starting from this page's first rank.
		$first = $page * $numPerPage + 1;
		$ranked = array();
		foreach(core_getuploads($args) as $i => $upload) {
			$ranked[$first + $i] = $upload;
		}
		hofList($ranked, $first, hofContextQuery($view, $year));
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
