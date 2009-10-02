<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		global $me;
		mustLogIn();

		$me = new User($_SESSION["userid"]);

		if(!array_key_exists("userid", $_REQUEST))
			$me->setPref("index", "thumbs");
	}

	function head() {
		echo <<<EOT
		<link id="gallery" rel="alternate" href="/offensive/pic_rss.php?gallery=true" type="application/rss+xml" title="PicLens feed" />
		<link rel="alternate" type="application/rss+xml" title="image stream" href="/offensive/pic_rss.php" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="/offensive/zip_rss.php" />
		<link href="/styles/grid.css?v=1.01" type="text/css" rel="stylesheet">
		<script type="text/javascript" src="/offensive/js/tmbolib.js?v=0.1"></script>
		<script type="text/javascript" src="/offensive/js/jquery-1.3.2.min.js"></script>
		<script type="text/javascript" src="/offensive/js/inview.js?v=1.12"></script>
    <script type="text/javascript" src="/offensive/js/lazyload.js?v=0.93"></script>
EOT;
	}

	function body() {
		$numPerPage = 100;

		$args = $_REQUEST;

		if(!array_key_exists("limit", $args)) {
			$args["limit"] = $numPerPage;
		}
		if(!array_key_exists("type", $args)) {
			$args["type"] = "image";
		}
		if($args["type"] == "topic") {
			trigger_error("topics do not have thumbnails", E_USER_ERROR);
		}
?>
<div class="heading">

    <?
		require("offensive/data/quips.inc");
		echo $quip;

		// employee of the month insertion
		$employee = get_include_path()."/offensive/employeeOfTheMonth.txt";
		if(file_exists($employee) && time() - filemtime($employee) < 172800) {
		    require("offensive/employeeOfTheMonth.txt");
		}
	?>

</div>

<?
	global $activeTab;
	$activeTab = "images";
	tabs();
?>

<div class="bluebox">
	<div id="grid-container">
		<div id="commands">
			<div class="floatleft"><? include( 'offensive/assets/pickupLink.inc' ) ?></div>
			<div class="floatright"><a href="./?c=main<?= query_string("c", "&") ?>">text view</a></div>
		</div>
		<div class="grid_thumbs" id="thumbnails">
			<ul>
			<?php
				$result = core_getuploads($args);
				$output = 0;

				foreach( $result as $upload ) {?>
				<li fileid="<?= $upload->id() ?>">
				  <div class="thumbcontainer">
					<div><a href="pages/pic.php?id=<?= $upload->id() ?>"
						<?php if($upload->filtered()) { ?>
							onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
						<? } ?>
					><img name="th<?= $upload->id()?>"
								src="<?= $upload->filtered() ? "/offensive/graphics/th-filtered.gif" : $upload->thumbURL() ?>"
								title="uploaded by <?= $upload->uploader()->username(); ?>"
						>
					</a></div>
					<div class="score"><a href="./?c=comments&amp;fileid=<?= $upload->id() ?>"><?= $upload->commentLabel(); ?></a></div>
			    </div>
			  </li>
		<?php } ?>
			</ul>
		</div>
		<div id="bottom">
			<p><a href="">load more…</a></p>
		</div>
	</div>
</div>
<?
}
?>
