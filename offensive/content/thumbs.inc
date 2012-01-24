<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
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

		// prevent changing preferred view if certain arguments are set
		if(!array_key_exists("userid", $_REQUEST))
			me()->setPref("index", "thumbs");
	}

	function head() {
		?>
		<link id="gallery" rel="alternate" href="<?= Link::rss("pic") ?>&gallery=true" type="application/rss+xml" title="PicLens feed" />
		<link rel="alternate" type="application/rss+xml" title="image stream" href="<?= Link::rss("pic") ?>" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="<?= Link::rss("zip") ?>" />
		<link href="/styles/grid.css?v=1.01" type="text/css" rel="stylesheet">
		<script type="text/javascript" src="/offensive/js/tmbolib.js?v=0.1"></script>
		<script type="text/javascript" src="/offensive/js/jquery-1.3.2.min.js"></script>
		<? if(count($_GET) == 1 && array_key_exists("c", $_GET)) { ?>
			<!-- lazyload_changes are only used if on a base page (?c=main not ?c=main&foo=bar) -->
			<script type="text/javascript">
				function html_upload(upload) {
					if(upload.type != "image") return "";
    	
				  // we cant find a thumbnail for this image.
				  if(typeof(upload.link_thumb) == "undefined") {
						upload.link_thumb = "/offensive/graphics/previewNotAvailable.gif";
					}
    		
				  // image is filtered
				  if(upload.filtered == true) {
				    orig_src = upload.link_thumb;
				    upload.link_thumb = "/offensive/graphics/th-filtered.gif";
				    mouseout = "onmouseout='changesrc(\"th"+upload.id+"\",\"/offensive/graphics/th-filtered.gif\")' onmouseover='changesrc(\"th"+upload.id+"\",\""+orig_src+"\")'";
				  } else {
				    mouseout = "";
				  }
    		
				  html = '<li style="display: none;" fileid="'+upload.id+'">'
				          +'<div class="thumbcontainer">'
				            +'<div>'
				              +'<a '+mouseout+' href="pages/pic.php?id='+upload.id+'">'
				                +'<img title="uploaded by '+upload.username+'" '
				                     +'src="'+upload.link_thumb+'" '
				                     +'name="th'+upload.id+'"/>'
				              +'</a>'
				            +'</div>'
				            +'<div class="score">'
				              +create_score(upload)
				            +'</div>'
				          +'</div>'
				        +'</li>';
    	
					return html;
				}
			</script>
    	<script type="text/javascript" src="/offensive/js/lazyload_changes.js?v=0.0.4"></script>';
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
		if($args["type"] == "topic") {
			trigger_error("topics do not have thumbnails", E_USER_ERROR);
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
				<div class="floatright"><a href="./?c=main<?= query_string("c", "&") ?>">text view</a></div>
			</div>
			<div class="grid_thumbs" id="thumbnails">
				<ul>
				<?php
					$result = core_getuploads($args);
					$revision = currentChange();
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
	<div class="blackbar"></div>
</div>
<script>
	var update_index = <?= $revision ?>;
</script>
<?
}
?>