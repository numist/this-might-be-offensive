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
		<?
		CSS::add("/styles/grid.css");
		JS::add("/offensive/js/pickuplink_visibility.js");
		JS::add("/offensive/js/lazyload_bottom.js");
	}

	function head_post_js() {
		?>
		<script type="text/javascript">
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				socket.on('comment', function(comment) {
					var old_row = $("li[fileid=" + comment.fileid +"]");
					if (old_row.length > 0) {
						if (comment.comment) {
							var comments_count = parseInt(old_row.find('.comments').text()) + 1;
							var label = comments_count == 1 ? 'comment' : 'comments';
							old_row.find('.comments').text(comments_count);
							old_row.find('.commentlabel').text(label);
							old_row.find('.score a').attr('title', comment.timestamp);
						}
						if (comment.vote) {
							var replace = comment.vote == 'this is good' ? '.goods' : '.bads';
							var vote_count = parseInt(old_row.find(replace).text()) + 1;
							old_row.find(replace).text(vote_count);
						}
						if (comment.tmbo) {
							var tmbo_span = old_row.find('.tmbos');
							if (tmbo_span.length == 0) {
								old_row.find('.score a').append(' x<span class="tmbos">0</span>');
								tmbo_span = old_row.find('.tmbos');
							}
							var tmbo_count = parseInt(tmbo_span.text()) + 1;
							tmbo_span.text(tmbo_count);
						}
					}
				});
				socket.on('image', function(image) {
					$('#thumbnails > ul').prepend(image.html.thumb);
				});
				socket.emit('subscribe', '/uploads/*');
			});
		</script>
		<?php
	}

	function body() {
		// TODO: remove in a couple of months, to give people a chance to re-set their keybindings.
		require("offensive/data/keynav.inc");
		$keysNeedUpdate = false;
		foreach($key_options as $option => $foo) {
			if($option == "noselect") continue;
			$val = me()->getPref($option);
			if($val) {
				$keys = unserialize($val);
				foreach($keys as $key) {
					if($key & KEY_META_AWARE == 0) {
						$keysNeedUpdate = true;
						break;
					}
				}
			}
			if($keysNeedUpdate) break;
		}
		if($keysNeedUpdate) {
			box("You're using custom key shortcuts which should be updated to be modifier-key-aware.<br />
			Please update them in your <a href=\"".Link::content("settings")."\">settings</a>.", "It'll only take a few seconds…");
		}
		
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
				<div class="floatleft"><? PickupLink::emit(); ?></div>
				<div class="floatright"><a href="<?= Link::content("main").query_string("c", "&") ?>">text view</a></div>
			</div>
			<div class="grid_thumbs" id="thumbnails">
				<ul>
				<?php
					$result = core_getuploads($args);
					$output = 0;
	
					foreach( $result as $upload ) {
						require("offensive/templates/thumbitem_file.inc");
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
<?
}
?>
