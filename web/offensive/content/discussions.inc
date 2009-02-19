<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	
	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
	}

	function body() {
		global $me;
		
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;		
		$args = $_REQUEST;
		$args["type"] = "topic";
		$args["p"] = $page;
	
?>
	<div class="heading">we need to talk.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<div style="font-weight:bold;text-align:right">
				<a href="./?c=newtopic">new topic</a>
			</div>
			<?
					$defaultSort = $me->getPref("sortorder_discussions") ?
					               $me->getPref("sortorder_discussions") :
					               "date_desc";
					if(strpos($defaultSort, "_") == false) $defaultSort = "date_desc";
					$args = $_REQUEST;
					if(!array_key_exists("sort", $args)) {
						$args["sort"] = $defaultSort;
					}
					
					$numPerPage = 100;
					$args = $_REQUEST;
					if(!array_key_exists("limit", $args)) {
						$args["limit"] = $numPerPage;
					}
					if(!array_key_exists("type", $args)) {
						$args["type"] = "topic";
					}
					
					$result = core_getuploads($args);
					
					// if we made it this far, sort is valid, so we don't have to check it
					if($args["sort"] != $defaultSort) {
						$me->setPref("sortorder_discussions", $args["sort"]);
					}
					
					?>
					<table width="100%" id="discussions">

						<tr style="background:#bbbbee;text-align:center;">
							<td><b><?= ($args["sort"] !== "date_desc"
								? '<a href="./?c=discussions&sort=date_desc">sort by thread creation date</a>'
								: 'sort by thread creation date' ) ?></b></td>
							<td><b><?= ($args["sort"] !== "comments_desc"
							    ? '<a href="./?c=discussions&sort=comments_desc">comment count</a>'
							    : 'comment count' )?></b></td>
							<td><b><?= ($args["sort"] !== "activity_desc"
							    ? '<b><a href="./?c=discussions&sort=activity_desc">latest comment</a></b>'
							    : 'latest comment' ) ?></b></td>
						</tr>

					<?
					
					foreach( $result as $upload ) {
						$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";

						?>
							<tr class="<?= $css ?>">
								<td><div class="clipr"><a href="./?c=comments&fileid=<?= $upload->id() ?>"><?= $upload->htmlFilename() ?></a></div></td>
								<td class="com_cnt"><a href="./?c=comments&fileid=<?= $upload->id() ?>">(<?=
									$upload->commentLabel(false);
									?>)</a></td>
								<td class="dater"><?= $upload->last_active() ?></td>
							</tr>
						<?
					}


					?>
						<tr>
							<td>
								<? if( $page > 0 ) { ?>
									<a href="<?= $_SERVER['PHP_SELF']?>?c=<?= $_REQUEST['c'] ?>&order=<?=$order?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
								<? } ?>
							</td>
							<td></td>
							<td style="text-align:right">
								<? if(count($result) == $numPerPage) { ?>
									<a href="<?= $_SERVER['PHP_SELF']?>?c=<?= $_REQUEST['c'] ?>&order=<?=$order?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
								<? } ?>
							</td>
						</tr>

				</table>
		</div>
<?
	}

	function discussionList() {
		global $me;

	}
?>