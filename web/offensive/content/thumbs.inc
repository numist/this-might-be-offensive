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
		<link href="/styles/thumbs.css" type="text/css" rel="stylesheet">
EOT;
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
	<div id="pickupbox"><? include( 'offensive/assets/pickupLink.inc' ) ?></div>
	<div id="textlink"><b><a href="./?c=main<?= query_string("c", "&") ?>">text view</a></b></div>
	<div id="thumbnails">
		<ul>
		<?php
			$result = core_getuploads($args);
			$output = 0;
			foreach( $result as $upload ) {	?>
				<li><div>
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
					<div><a href="./?c=comments&amp;fileid=<?= $upload->id() ?>"><?= $upload->commentLabel(); ?></a></div>
			</div>	</li>
		<?php } ?>
		</ul>	
	</div>
	<span id="prevpage">&nbsp;
	<?
	if( $page > 0 ) { ?>
	<a href="./?<?= query_string("p") ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
	<? } ?>
	</span>
	<span id="nextpage">&nbsp;
	<? if(count($result) == $numPerPage) { ?>
		<a href="./?<?= query_string("p") ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
	<? } ?>
	</span>
</div>
<?
}
?>
