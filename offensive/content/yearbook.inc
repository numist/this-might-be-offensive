<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/classes.inc' );
	require_once( 'offensive/assets/core.inc' );
	require_once('offensive/assets/pickupLink.inc');

	function start() {
		mustLogIn();

		// support for conditional GETs, since the yearbook doesn't change often
		$sql = "SELECT timestamp FROM offensive_uploads	WHERE type='avatar'
		        ORDER BY timestamp DESC LIMIT 1";
		$result = tmbo_query($sql);
		$tstamp = mysql_fetch_array($result);
		require_once("offensive/assets/conditionalGet.inc");
		conditionalGet(strtotime($tstamp[0]));
	}

	function head() {
		?>
		<link href="/styles/grid.css?v=1.02" type="text/css" rel="stylesheet">
		<script type="text/javascript" src="/offensive/js/jquery-1.7.1.min.js"></script>
		<script type="text/javascript" src="/offensive/js/pickuplink_visibility.js?v=0.0.2"></script>
		<!-- lazyload_bottom -->
		<script type="text/javascript" src="/offensive/js/lazyload_bottom.js?v=0.0.5"></script>
		<?
	}

	function body() {
?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">the beautiful people.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? yearbook(); ?>
		</div>
		<div class="blackbar"></div>
	</div>
<?
	}

	function yearbook() {
		// get the default from user prefs
		$defaultSort = me()->getPref("sortorder_yearbook") ?
		               me()->getPref("sortorder_yearbook") :
		               "uname_asc";

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;

		$THUMBS_PER_ROW = 4;
		$count = 0;

		$args = $_REQUEST;
		// handle the outdated option "date", superceded by "date_desc"
		if((array_key_exists("sort", $args) && $args["sort"] == "date") || $defaultSort == "date") {
			$args["sort"] = "date_desc";
		}

		if(!array_key_exists("sort", $args)) {
			$args["sort"] = $defaultSort;
		}
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = "100";
		}

		$result = core_getyearbook($args);

		// if we made it this far, sort is valid, so we don't have to check it
		if($args["sort"] != $defaultSort) {
			me()->setPref("sortorder_yearbook", $args["sort"]);
		}
?>
		<div id="grid-container">
			<div id="commands">
				<div class="floatleft"><? print_pickuplink() ?></div>
				<div class="floatright">
					<? if($args["sort"] !== 'uname_asc') { ?>
					<a href="./?c=yearbook&sort=uname_asc">sort by username</a>
					<? } ?>
					<? if($args["sort"] !== 'date_desc') { ?>
						<a href="./?c=yearbook&sort=date_desc">sort by date</a>
					<? } ?>
				</div>
			</div>
			<div class="grid_thumbs" id="yearbook">
				<ul>
			<?

		// output loop
		foreach($result as $upload) {
			?>
					<li fileid="<?= $upload->id() ?>">
						<div class="thumbcontainer">
							<div><a href="pages/pic.php?id=<?= $upload->id() ?>"
								<? if($upload->filtered()) { ?>
								onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
								onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
								<? } ?>
							><img name="th<?= $upload->id()?>"
								src="<?= $upload->filtered()
									? "/offensive/graphics/th-filtered.gif"
									: $upload->thumbURL() ?>"
								>
							</a></div>
							<div class="score">
								<a href="./?c=user&userid=<?= $upload->uploader()->id() ?>"><?= $upload->uploader()->username() ?></a>
								<a href="./?c=comments&fileid=<?= $upload->id() ?>">[c]</a>
							</div>
						</div>
					</li>
				<?php } ?>
				</ul>
			</div>
			<div id="bottom">
				<?
					// next page!
					$args = $_GET;
					if(array_key_exists("p", $args)) {
						$args["p"]++;
					} else {
						$args["p"] = 1;
					}
				?>
				<p id="morelink"><a href="<?= $_SERVER['PHP_SELF'] ?>?<?= http_build_query($args) ?>">more…</a></p>
			</div>
		</div>
<?
	}
?>