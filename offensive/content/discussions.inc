<?php
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
		JS::add("/offensive/js/prep_evenodd.js");
		JS::add("/offensive/js/lazyload_bottom.js");
		?>
		<link rel="alternate" type="application/rss+xml" title="discussions" href="<?= Link::rss("discuss") ?>" />
		<?php
	}

	function head_post_js() {
		$sort = array_key_exists("sort", $_REQUEST) ? $_REQUEST["sort"] : defaultSort();
		$firstPage = !(isset($_REQUEST["p"]) && $_REQUEST["p"] > 0);
		?>
		<script type="text/javascript">
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				var sort = <?= json_encode($sort) ?>;
				var firstPage = <?= $firstPage ? "true" : "false" ?>;
				var list = document.querySelector('#discussions ul');

				// a new comment: bump the count, point the activity line at it,
				// and move the row to the top when the list is sorted by it.
				socket.on('comment', function(comment) {
					var row = list.querySelector('li[data-fileid="' + comment.fileid + '"]');
					if(!row) return;

					if(comment.comment) {
						var count = row.querySelector('.comments');
						var n = parseInt(count.textContent, 10) + 1;
						count.textContent = n;
						row.querySelector('.commentlabel').textContent = n == 1 ? 'comment' : 'comments';
					}

					var link = document.createElement('a');
					link.href = '<?= Link::content("comments") ?>&fileid=' + comment.fileid + '#' + comment.id;
					link.textContent = 'commented less than a minute ago';
					var time = row.querySelector('time');
					time.dateTime = comment.timestamp;
					time.replaceChildren(comment.username + ' ', link);

					if(sort == 'activity_desc' && firstPage) {
						list.prepend(row);
					}
				});

				// a new topic is the newest and most recently active row.
				socket.on('topic', function(topic) {
					if(!firstPage || (sort != 'date_desc' && sort != 'activity_desc')) return;
					if(me.squelched.some(function(id) { return id == topic.userid; })) return;
					list.insertAdjacentHTML('afterbegin', topic.html);
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
	<div class="heading">we need to talk.</div>
<?php
		tabs();
?>
		<div class="bluebox">
			<div id="grid-container">
				<div id="commands">
					<div>
						<a href="<?= Link::content("newtopic") ?>">new topic</a>
					</div>
				</div>
<?php

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
				<div class="header">
					<div>sort by</div>
					<div>
							<?= ($args["sort"] !== "date_desc"
										? '<a href="./?'.query_string("sort").'&sort=date_desc">creation date</a>'
										: 'thread creation date' ) ?>
							<?= ($args["sort"] !== "comments_desc"
										? '<a href="./?'.query_string("sort").'&sort=comments_desc">comment count</a>'
										: 'comment count' )?>
							<?= ($args["sort"] !== "activity_desc"
										? '<b><a href="./?'.query_string("sort").'&sort=activity_desc">latest comment</a></b>'
										: 'latest comment' ) ?>
					</div>
				</div>
				<div class="grid" id="discussions">
					<ul>
<?php
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
					<?php
						// next page!
						$args = $_GET;
						if(array_key_exists("p", $args)) {
							$args["p"]++;
						} else {
							$args["p"] = 1;
						}
					?>
					<p id="morelink"><a href="<?= $_SERVER['SCRIPT_NAME'] ?>?<?= http_build_query($args) ?>">more…</a></p>
				</div>
			</div>
		</div>
	</div>
<?php
	}
	
	function defaultSort() {
		$defaultSort = me()->getPref("sortorder_discussions")
										? me()->getPref("sortorder_discussions")
										: "date_desc";
		if(strpos($defaultSort, "_") == false) return "date_desc";
		return $defaultSort;
	}
?>
