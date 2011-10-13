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
		echo '
		<link id="gallery" rel="alternate" href="'.Link::rss("audio").'" type="application/rss+xml" title="audio stream" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="'.Link::rss("zip").'" />
		<link href="/styles/grid.css?v=1.03" type="text/css" rel="stylesheet">';
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
		<div class="grid" id="audio">
			<ul>
<?
			$result = core_getuploads($args);
			foreach( $result as $upload ) {
				$css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
?>
				<li>
					<div class="col col1 <?= $css ?>">
						<a href="pages/pic.php?id=<?= $upload->id() ?>" class="<?= $css ?>" title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>"><?= $upload->htmlFilename() ?></a>
						<? if(!file_exists($upload->file())) { echo "(expired)"; } ?>
					</div>
					<div class="col col2 <?= $css ?>">
						<a href="./?c=comments&fileid=<?= $upload->id() ?>" class="<?= $css ?>"><nobr><?= $upload->commentLabel(); ?></nobr></a>
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
			<? if(count($result) == $numPerPage) { ?>
				<p id="nextpage">
					<a href="./?<?= query_string("p") ?>&p=<?= $page + 1 ?>">next page</a> &raquo;
				</p>
			<? } ?>
			</div>
		</div>
	</div>
</div>

<?
}
?>
