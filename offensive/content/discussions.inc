<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
	}

	function head() {
		CSS::add("/styles/grid.css");
		JS::add("/offensive/js/prep_evenodd.js");
		JS::add("/offensive/js/lazyload_bottom.js");
		?>
		<link rel="alternate" type="application/rss+xml" title="discussions" href="<?= Link::rss("discuss") ?>" />
		<?
	}

	function head_post_js() {
		?>
		<script type="text/javascript">
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				function recolorRows() {
					$('#discussions li').not('.header').each( function(idx) {
						$(this).find('.col').removeClass('odd_row even_row').addClass( (idx % 2 == 0) ? 'even_row' : 'odd_row');
					});
				}
				
				var _sortOrder;
				function sortOrder() {
					if(!_sortOrder) {
						$('#discussions .header .col').each(function(idx) {
							if ($(this).find('a').length == 0)
								_sortOrder = idx;
						});
					}
					return _sortOrder;
				}

				socket.on('comment', function(comment) {
					var old_row = $("li[fileid=" + comment.fileid +"]");
					if (old_row.length > 0) {
						var count = parseInt(old_row.find('.comments').text()) + 1;
						var label = count == 1 ? 'comment' : 'comments';
						old_row.find('.comments').text(count);
						old_row.find('.commentlabel').text(label);
						old_row.find('.col3').text(comment.timestamp);
						// if (sortOrder() == 2) {
						//   $('#discussions .header').after(old_row);
						//   recolorRows();
						// }
					}
				});
				socket.on('topic', function(topic) {
					if(sortOrder() != 1) {
						$('#discussions .header').after(topic.html);
						recolorRows();
					}
				});
				socket.emit('subscribe', '/uploads/*');
			});
		</script>
		<?php
	}

	function body() {
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$args = $_REQUEST;
		$args["type"] = "topic";
		$args["p"] = $page;

?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">we need to talk.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<div id="grid-container">
				<div id="commands">
					<div class="floatright">
						<a href="<?= Link::content("newtopic") ?>">new topic</a>
					</div>
				</div>
<?

			$args = $_REQUEST;
			if(!array_key_exists("sort", $args)) {
				$args["sort"] = me()->getPref("sortorder_discussions");
			}

			$numPerPage = 100;
			if(!array_key_exists("limit", $args)) {
				$args["limit"] = $numPerPage;
			}
			if(!array_key_exists("type", $args)) {
				$args["type"] = "topic";
			}

			// if any of the args have incorrect values, core_getuploads will kill us here
			$result = core_getuploads($args);

			// if we made it this far, sort is valid, so we don't have to check it
			if($args["sort"] != me()->getPref("sortorder_discussions")) {
				me()->setPref("sortorder_discussions", $args["sort"]);
			}
?>
				<div class="grid" id="discussions">
					<ul>
						<li class="header">
							<div class="col col1">
									<?= ($args["sort"] !== "date_desc"
												? '<a href="./?'.query_string("sort").'&sort=date_desc">sort by thread creation date</a>'
												: 'sort by thread creation date' ) ?>
							</div>
							<div class="col col2">
									<?= ($args["sort"] !== "comments_desc"
												? '<a href="./?'.query_string("sort").'&sort=comments_desc">comment count</a>'
												: 'comment count' )?>
							</div>
							<div class="col col3">
									<?= ($args["sort"] !== "activity_desc"
												? '<b><a href="./?'.query_string("sort").'&sort=activity_desc">latest comment</a></b>'
												: 'latest comment' ) ?>
							</div>
						</li>
<?
						$pickuplinks = PickupLink::get();
						$first = array_key_exists("p", $args) && $args["p"] > 0 ? false : true;
						foreach( $result as $upload ) {
							$css = (isset($css) && strpos($css, "even_row") !== false) ? "odd_row" : "even_row";
							if(!$first && $args["sort"] == "date_desc"
							&& in_array($upload->id(), $pickuplinks)) {
								$css .= " hilight_row";
							}
							require("offensive/templates/listitem_topic.inc");
							
							$first = false;
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
	
	function defaultSort() {
		$defaultSort = me()->getPref("sortorder_discussions")
										? me()->getPref("sortorder_discussions")
										: "date_desc";
		if(strpos($defaultSort, "_") == false) return "date_desc";
		return $defaultSort;
	}
?>
