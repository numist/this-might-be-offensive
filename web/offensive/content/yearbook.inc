<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	
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
		               "username";

		// see if we're trying to change it
		$order = array_key_exists("sort", $_REQUEST) ? $_REQUEST['sort'] : $defaultSort;
		$req = array_key_exists("sort", $_REQUEST) ? $_REQUEST['sort'] : '';
		
		// set up the query ordering
		$sort = ($order == 'date' ? 'up.timestamp DESC' : 'username');
		
		// if the sort order was changed, save it
		if($order != $defaultSort) {
			$me->setPref("sortorder_yearbook", $order);
		}

		if($req == 'date') {
			?><b><a href="./?c=yearbook">sort by username</a></b><?
		} else {
			?><b><a href="./?c=yearbook&sort=date">sort by date</a></b><?
		}
?>



<table width="100%" class="thumbnails">

<?

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;

		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT users.userid, username AS user_username, up.*
					FROM users, offensive_uploads up
					WHERE users.userid=up.userid
						AND up.type='avatar'
						AND account_status != 'locked'
						AND status != 'pending'
						AND users.timestamp > DATE_SUB( NOW(), INTERVAL 12 MONTH )
						AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up WHERE type='avatar' AND userid=users.userid)
					ORDER BY $sort";

		$result = tmbo_query($sql);

		$THUMBS_PER_ROW = 4;
		$count = 0;

		$me = me();

		while( $row = mysql_fetch_assoc( $result ) ) {
			$upload = new Upload($row);
			
			if($upload->expired() || ($me && $me->squelched($upload->uploader()))) continue;
		
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
						<a href="./?c=user&userid=<?= $upload->uploader()->userid() ?>"><?= $upload->uploader()->username() ?></a>
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

	</table>


<?
	}
?>