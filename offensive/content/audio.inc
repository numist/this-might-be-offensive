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
		CSS::add("/styles/grid.css");
		JS::add("/offensive/js/pickuplink_visibility.js");
		JS::add("/offensive/js/prep_evenodd.js");
		JS::add("/offensive/js/lazyload_bottom.js");
		?>
		<link id="gallery" rel="alternate" href="<?= Link::rss("audio") ?>" type="application/rss+xml" title="audio stream" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="<?= Link::rss("zip") ?>" />
		<?
	}

	function head_post_js() {
		?>
		<script type="text/javascript">
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				function recolorRows() {
					$('#main li').not('.header').each( function(idx) {
						// Apparently main is 1-index based
						$(this).find('.col').removeClass('odd_row even_row').addClass( (idx % 2 == 0) ? 'odd_row' : 'even_row');
					});
				}
				
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
				socket.on('audio', function(audio) {
					$('#main > ul').prepend(audio.html.list);
					recolorRows();
				});
				socket.emit('subscribe', '/uploads/*');
			});
		</script>
		<?php
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
					<? PickupLink::emit(); ?>
				</div>
				<div class="floatright">&nbsp;</div>
			</div>
			<div class="grid" id="main">
				<ul>
	<?
				$result = core_getuploads($args);
				foreach( $result as $upload ) {
					$css = (isset($css) && strpos($css, "odd_row") !== false) ? "even_row" : "odd_row";
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
<?
}
?>
