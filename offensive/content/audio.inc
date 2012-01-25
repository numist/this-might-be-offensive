<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
	}

	function head() {
		?>
		<link id="gallery" rel="alternate" href="<?= Link::rss("audio") ?>" type="application/rss+xml" title="audio stream" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="<?= Link::rss("zip") ?>" />
		<link href="/styles/grid.css?v=1.03" type="text/css" rel="stylesheet">
		<script type="text/javascript" src="/offensive/js/tmbolib.js?v=0.0.3"></script>
		<script type="text/javascript" src="/offensive/js/jquery-1.7.1.min.js"></script>
		<? if(count($_GET) == 1 && array_key_exists("c", $_GET)) { ?>
			<!-- lazyload_changes are only used if on a base page (?c=main not ?c=main&foo=bar) -->
    	<script type="text/javascript" src="/offensive/js/lazyload_changes.js?v=0.0.11"></script>
		<? } ?>
		<script type="text/javascript">
			function prep_item(item, neighbor) {
				var color_class = (neighbor.find('.odd_row').length > 0) ? "even_row" : "odd_row";
				$(item).find('div.col').removeClass("odd_row even_row").addClass(color_class);
				return item;
			}
		</script>
		<script type="text/javascript" src="/offensive/js/lazyload_bottom.js?v=0.0.2"></script>
		<?
	}

	function body() {
		$numPerPage = 100;

		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$args = $_REQUEST;

		if(!array_key_exists("limit", $args)) {
			$args["limit"] = $numPerPage;
		}
		if(!array_key_exists("type", $args)) {
			$args["type"] = "audio";
		}

?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">
		oh lookie here...
	</div>
	
	<?
		global $activeTab;
		$activeTab = "audio";
		tabs();
	?>

	<div class="bluebox">
		<div id="grid-container">
			<div id="commands">
				<div class="floatleft">
					<? require( 'offensive/assets/pickupLink.inc' ) ?>
				</div>
			</div>
			<div class="grid" id="main">
				<ul>
	<?
				$result = core_getuploads($args);
				$revision = currentChange();
				foreach( $result as $upload ) {
					$css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
					require("offensive/templates/listitem_file.inc");
	 			} ?>
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
	</div>
	<div class="blackbar"></div>
</div>
<script>
	var update_index = <?= $revision ?>;
</script>
<?
}
?>
