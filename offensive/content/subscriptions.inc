<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );

	function title() {
		global $filename;
		return "[ tmbo ] : subscriptions";
	}

	function start() {
		mustLogIn();
	}

	function head() {
		echo <<<EOT
		<link href="/styles/grid.css?v=1.02" type="text/css" rel="stylesheet">
EOT;
	}

	function body() {
		$usrid = me()->id();

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;

		$sql = "SELECT DISTINCT s.fileid, u.filename
			FROM offensive_subscriptions s,
				offensive_uploads u
			WHERE s.userid=$usrid 
				AND s.fileid=u.id
			ORDER BY s.fileid DESC
			LIMIT $start, $numPerPage";

		$result = tmbo_query( $sql );
?>
	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading">
			subscriptions.
		</div>

<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>

		<div class="bluebox">
			<div id="grid-container">
				<div class="grid" id="subscriptions">
					<ul>
<?
		while( $row = mysql_fetch_assoc( $result ) ) {
			$row['filename'] = htmlEscape($row['filename']);
			$css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
			?>
						<li class="grid_row" fileid="<?= $row['fileid'] ?>">
							<div class="col col1 <?= $css ?>">
								<a href="./?c=comments&fileid=<?= $row['fileid'] ?>"><?= $row['filename'] ?></a>
							</div>
							<div class="col col2 <?= $css ?>">
								<a href="subscribe.php?un=1&fileid=<?= $row['fileid'] ?>" title="take this thread off my 'unread comments' watch list.">unsubscribe</a>
							</div>
						</li>
			<? } ?>
					</ul>
					<div id="navigation">
					<? if( $page > 0 ) { ?>
						<p id="prevpage">
							<a href="./?<?= query_string("p") ?>&p=<?= $page - 1 ?>">&laquo; previous page</a>
						</p>
					<? } ?>
					<? if(mysql_num_rows($result) == $numPerPage) { ?>
						<p id="nextpage">
							<a href="./?<?= query_string("p") ?>&p=<?= $page + 1 ?>">next page</a> &raquo;
						</p>
						<? } ?>
					</div>
				</div>
			</div>
		</div>
		<div class="blackbar"></div>
	</div>
	<?
	}
?>