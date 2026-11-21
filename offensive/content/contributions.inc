<?php
	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/core.inc' );
	require_once( 'offensive/assets/contributions.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	// every upload of one user, a page at a time, newest first or by votes.

	function start() {
		mustLogIn();

		if(!isset($_REQUEST['userid']) || !is_intger($_REQUEST['userid'])) {
			header("Location: ./");
			exit;
		}

		global $them;
		$them = new User($_REQUEST['userid']);

		if(!$them->exists()) {
			header("Location: ./");
			exit;
		}
	}

	function head() {
		JS::add("/offensive/js/contributions_more.js");
	}

	function title() {
		global $them;
		return "[contributions] : ".$them->username();
	}

	function body() {
		global $them;
		$usrid = $them->id();
		$sort = contributionsSort();
		$thumbs = contributionsThumbs();
		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ? max(0, (int)$_REQUEST['p']) : 0;
?>
<div class="contentbox">
	<div class="heading"><?= $them->htmlUsername() ?>'<?= substr($them->username(), -1) == "s" ? "" : "s" ?> contributions to society.</div>
	<?php tabs(); ?>
	<div class="bluebox<?= $thumbs ? " grid_thumbs" : "" ?>">
<?php
		if(me()->squelched($them)) {
			?><p>You've squelched <?= $them->htmlUsername() ?>.</p><?php
		} else {
			contributionsSortLinks($usrid, $sort, $thumbs);
			// one more than a page, to know whether there is a next one.
			$uploads = contributionsQuery($usrid, $sort, $numPerPage + 1, $page * $numPerPage, $thumbs);
			$more = count($uploads) > $numPerPage;
			// a page can start partway through a year; its heading repeats.
			// contributions_more.js folds such a page into the year above it.
			contributionsList(array_slice($uploads, 0, $numPerPage), $sort != "votes", contributionsContextQuery($usrid, $sort, $thumbs), $thumbs);
			if($more) {
?>
		<p id="morelink"><a href="<?= contributionsLink($usrid, $sort, $thumbs) ?>&p=<?= $page + 1 ?>">more…</a></p>
<?php
			}
		}
?>
	</div>
</div>
<?php
	}
?>