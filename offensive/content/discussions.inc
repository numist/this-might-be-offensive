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
		?>
		<link rel="alternate" type="application/rss+xml" title="discussions" href="<?= Link::rss("discuss") ?>" />
		<link href="/styles/grid.css?v=1.02" type="text/css" rel="stylesheet">
		<script type="text/javascript" src="/offensive/js/tmbolib.js?v=0.1"></script>
		<script type="text/javascript" src="/offensive/js/jquery-1.7.1.min.js"></script>
		<? if(count($_GET) == 1 && array_key_exists("c", $_GET) && defaultSort() == "date_desc") { ?>
			<!-- lazyload_changes are only used if on a base page (?c=main not ?c=main&foo=bar) -->
			<script type="text/javascript">
				function html_upload(upload) {
					if(upload.type != "topic") return "";

				  var color_class = ($('li[fileid]').first().find('.odd_row').length > 0) ? "even_row" : "odd_row";
				
					html = '<li fileid="'+upload.id+'">'
						      +'<div class="col col1 '+color_class+'">'
						      	+'<a href="./?c=comments&fileid='+upload.id+'">'+upload.filename.htmlescape()+'</a>'
						      +'</div>'
						      +'<div class="col col2 '+color_class+'">'
										+'<span class="score">'
						      		+'<a href="./?c=comments&fileid='+upload.id+'">'
								  			+'(';
					if(upload.comments == 1) {
						html        	+='<span class="comments">1</span> <span class="commentlabel">comment</span>';
					} else {
						html        	+='<span class="comments">'+upload.comments+'</span> <span class="commentlabel">comments</span>';
					}
					html					+=')'
											+'</a>'
										+'</span>'
						      +'</div>'
						      +'<div class="col col3 '+color_class+'">'
						      		+upload.last_active
						      +'</div>'
						    +'</li>';
				  
			    return html;
				}
			</script>
    	<script type="text/javascript" src="/offensive/js/lazyload_changes.js?v=0.0.9"></script>
			<?
		}?>
		<!-- lazyload_bottom -->
		<script type="text/javascript">
			function prep_item(item, neighbor) {
				var color_class = (neighbor.find('.odd_row').length > 0) ? "even_row" : "odd_row";
				$(item).find('div.col').removeClass("odd_row even_row").addClass(color_class);
				return item;
			}
		</script>
		<script type="text/javascript" src="/offensive/js/lazyload_bottom.js?v=0.0.1"></script>
		<?
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
						<a href="./?c=newtopic">new topic</a>
					</div>
				</div>
<?

			$args = $_REQUEST;
			if(!array_key_exists("sort", $args)) {
				$args["sort"] = defaultSort();
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
			$revision = currentChange();

			// if we made it this far, sort is valid, so we don't have to check it
			if($args["sort"] != defaultSort()) {
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
						foreach( $result as $upload ) {
							$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
?>
						<li fileid="<?= $upload->id() ?>">
							<div class="col col1 <?= $css ?>">
								<a href="./?c=comments&fileid=<?= $upload->id() ?>"><?= $upload->htmlFilename() ?></a>
							</div>
							<div class="col col2 <?= $css ?>">
								<span class="score">
									<a href="./?c=comments&fileid=<?= $upload->id() ?>">(<?=$upload->commentLabel(false);?>)</a>
								</span>
							</div>
							<div class="col col3 <?= $css ?>">
									<?= $upload->last_active() ?>
							</div>
						</li>
						<? } ?>
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
	
	function defaultSort() {
		$defaultSort = me()->getPref("sortorder_discussions")
										? me()->getPref("sortorder_discussions")
										: "date_desc";
		if(strpos($defaultSort, "_") == false) return "date_desc";
		return $defaultSort;
	}
?>