<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
		
		if(date("m-d") == "04-01") {
			//header("Location: …");
			//exit;
		}

		me()->setPref("index", "");
	}

	function head() {
		?>
		<link id="gallery" rel="alternate" href="<?= Link::rss("pic") ?>&gallery=true" type="application/rss+xml" title="PicLens feed" />
		<link rel="alternate" type="application/rss+xml" title="image stream" href="<?= Link::rss("pic") ?>" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="<?= Link::rss("zip") ?>" />
		<link href="/styles/grid.css" type="text/css" rel="stylesheet" />
		<script type="text/javascript" src="/offensive/js/tmbolib.js?v=0.1"></script>
		<script type="text/javascript" src="/offensive/js/jquery-1.3.2.min.js"></script>
		<? if(count($_GET) == 1 && array_key_exists("c", $_GET)) { ?>
			<!-- lazyload_changes are only used if on a base page (?c=main not ?c=main&foo=bar) -->
			<script type="text/javascript">
				function html_upload(upload) {
					if(upload.type != "image") return "";
    	
				  var color_class = ($('li[fileid]').first().find('.odd_row').length > 0) ? "even_row" : "odd_row";
				  html = '<li style="display: none;" fileid="'+upload.id+'">'
				          +'<div class="col col1 '+color_class+'">'
				            +'<a href="pages/pic.php?id='+upload.id+'" title="uploaded by '+upload.username.htmlescape()+'">';
					if(upload.nsfw || upload.tmbo) {
						html     +='<span class="tags">';
						if(upload.nsfw) {
							html     +='[nsfw]';
						}
						if(upload.tmbo) {
							html     +='[tmbo]';
						}
						html     +='</span> ';
					}
				  html        +=upload.filename.htmlescape()
				            +'</a>'
				          +'</div>'
				          +'<div class="col col2 '+color_class+'">'
				            +'<span class="score">'
				              +create_score(upload)
				            +'</span>'
				          +'</div>'
				        +'</li>';
				  
			    return html;
				}
			</script>
			<?
		}
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

?>
<div class="contentbox">
<div class="blackbar"></div>
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
			<div class="floatright"><a href="./?c=thumbs<?= query_string("c p", "&") ?>">thumbnail view</a></div>
		</div>
		<div class="grid" id="main">
			<ul>
			<?php
				$result = core_getuploads($args);
				$revision = currentChange();
				$output = 0;
				foreach( $result as $upload ) {
					$css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
					$filename = $upload->htmlFilename();
			?>
				<li fileid="<?= $upload->id() ?>">
					<div class="col col1 <?= $css ?>">
						<a href="pages/pic.php?id=<?= $upload->id() ?>" title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>"><?= $upload->htmlFilename() ?></a>
					</div>
					<div class="col col2 <?= $css ?>">
						<span class="score"><a href="./?c=comments&fileid=<?= $upload->id() ?>"><?= $upload->commentLabel(); ?></a></span>
					</div>
				</li>
			<?
				}
			?>
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
