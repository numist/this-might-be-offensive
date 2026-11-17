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
	function body() {

		$view = isset( $_REQUEST['t'] ) ? sqlEscape( $_REQUEST['t'] ) : 'hof';	
?>
<div class="contentbox">
	<div class="heading">hall of fame: the coolest files of all time (according to you).</div>
	<?php tabs(); ?>

	<div class="tabs subtabs">
		<div class="<?= ($view == 'hof' ? 'tabon' : 'taboff') ?>"><a href="<?= Link::content("hof") ?>">all time</a></div>
		<div class="<?= ($view == 'today' ? 'tabon' : 'taboff') ?>"><a href="<?= Link::content("hof") ?>&t=today">past 24 hours</a></div>
	</div>


	<div class="bluebox">
		<div id="grid-container">
			<div class="grid" id="hof">
<?php
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
		if(!array_key_exists("type", $args)) {
			$args["type"] = "image";
		}

		$result = core_getuploads($args);
		// the list numbers the rows, starting from this page's first rank.
		// the rows are grid boxes, not list items, so browsers don't apply
		// start and value to them; the inline counter styles do that.
		$count = $page * $numPerPage;
		$skipped = false;
?>
				<ol start="<?= $count + 1 ?>" style="counter-reset: list-item <?= $count ?>">
<?php
		foreach($result as $upload) {
			++$count;
			// cellophane doesn't want her images in the hof;
			// this is a quick hack to omit them. the next row says its
			// rank, so the list doesn't renumber around the gap.
			if( $upload->uploader()->id() == 1138 ) {
				$skipped = true;
				continue;
			}
			$value = $skipped ? ' value="'.$count.'" style="counter-set: list-item '.$count.'"' : "";
			$skipped = false;
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
