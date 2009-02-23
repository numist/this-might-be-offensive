<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/classes.inc' );
	require_once( 'offensive/assets/core.inc' );
	
	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
		
		// support for conditional GETs, since the yearbook doesn't change often
		$sql = "SELECT timestamp FROM offensive_uploads	WHERE type='avatar'
		        ORDER BY timestamp DESC LIMIT 1";
		$result = tmbo_query($sql);
		$tstamp = mysql_fetch_array($result);
		require_once("offensive/assets/conditionalGet.inc");
		conditionalGet(strtotime($tstamp[0]));
	}

	function body() {
?>
	<div class="heading">the beautiful people.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? yearbook(); ?>
		</div>
<?
	}

	function yearbook() {
		global $me;
		
		// get the default from user prefs
		$defaultSort = $me->getPref("sortorder_yearbook") ?
		               $me->getPref("sortorder_yearbook") :
		               "uname_asc";

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;

		$THUMBS_PER_ROW = 4;
		$count = 0;

		$args = $_REQUEST;
		// handle the outdated option "date", superceded by "date_desc"
		if($args["sort"] == "date" || $defaultSort == "date") {
			$args["sort"] = "date_desc";
		}
		
		if(!array_key_exists("sort", $args)) {
			$args["sort"] = $defaultSort;
		}
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = "100";
		}

		$result = core_getyearbook($args);
		
		// if we made it this far, sort is valid, so we don't have to check it
		if($args["sort"] != $defaultSort) {
			$me->setPref("sortorder_yearbook", $args["sort"]);
		}

		// sorting links
		if($args["sort"] !== 'uname_asc') {
			?><b><a href="./?c=yearbook&sort=uname_asc">sort by username</a></b> <?
			if($args["sort"] !== 'date_desc') {
				?>| <?
			}
		}
		if($args["sort"] !== 'date_desc') {
			?><b><a href="./?c=yearbook&sort=date_desc">sort by date</a></b> <?
		}
		?>
		<table width="100%" class="thumbnails">
		<?

		// output loop
		foreach($result as $upload) {
			if( (($count++) % $THUMBS_PER_ROW) == 0 ) {
				$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
				?><tr class="<?= $css ?>"><?
			}
		
			?>
				
					<td>
						<? // forward-compatible image filtering? oh yes, squelching… ?>
						<a href="pages/pic.php?id=<?= $upload->id() ?>"
							<? if($upload->filtered()) { ?>
								onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
						 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
							<? } ?>
						><img name="th<?= $upload->id()?>"
							src="<?= $upload->filtered()
								? "/offensive/graphics/th-filtered.gif" 
								: $upload->thumbURL() ?>"
						/></a>
						
						<br/>
						<a href="./?c=user&userid=<?= $upload->uploader()->id() ?>"><?= $upload->uploader()->username() ?></a>
						<a href="./?c=comments&fileid=<?= $upload->id() ?>">[c]</a>						
					</td>

			<?
			
			if( (($count) % $THUMBS_PER_ROW) == 0 ) {
				?></tr><?
			}
		}

		if( ($count % $THUMBS_PER_ROW) != 0 ) {
			?></tr><?
		}
		?>

		<tr>
			<td>
				<? if( $page > 0 ) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
				<? } ?>
			</td>
			<td colspan="2"></td>
			<td style="text-align:right">
				<? if(count($result) == $numPerPage) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
				<? } ?>
			</td>
		</tr>
	</table>
<?
	}
?>