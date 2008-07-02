<?

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once('offensive/assets/tabs.inc');
	require_once('offensive/assets/functions.inc');
	
	function start() {
		mustLogIn();
	}

	function body() {

		$view = isset( $_REQUEST['t'] ) ? sqlEscape( $_REQUEST['t'] ) : 'hof';	
?>

		<div class="heading">hall of fame: the coolest files of all time (according to you sick puppies).</div>
		<? tabs(); ?>

		<div style="padding-top:8px;background:#ccccff;background-image:url( graphics/subtab_bg.gif );background-position:top left;background-repeat:repeat-x">
			<div class="<?= ($view == 'hof' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof">all time</a></div>
			<div class="<?= ($view == 'today' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof&t=today">past 24 hours</a></div>
			<div class="tabspacer" style="background:none">&nbsp;</div>
		</div>

				
		<div class="bluebox">

			<table width="100%">

			<tr class="<? $css = "evenfile"; echo $css ?>">
				<td style="text-align:right"><b>rank</b></td>
				<td><b>filename</b></td>
				<td><b>uploaded&nbsp;by</b></td>
				<td><b>comments</b></td>
				<td><b>votes</b></td>
				<td><b><? if($view=="today") { ?> hours <? } else { ?> weeks <? } ?></b></td> 
			</tr>

<?
		$sql = "SELECT up.id as fileid, cache.good as votes, up.userid, up.filename, up.timestamp, users.username, up.nsfw, up.tmbo, up.type, cache.comments FROM offensive_uploads up, users, offensive_count_cache cache WHERE up.id = cache.threadid AND users.userid = up.userid AND up.type = 'image'";
		if($view == "today") $sql .= " AND up.timestamp > now() - INTERVAL 1 DAY";
		if(isset($_REQUEST['nonsfw'])) $sql .= " AND up.nsfw = 0";
		if(isset($_REQUEST['notmbo'])) $sql .= " AND up.tmbo = 0";
		$sql .=" ORDER BY cache.good DESC LIMIT 100";

		$result = tmbo_query( $sql );
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
			if( $row['userid'] == $_SESSION['userid'] ) {
				$css = "my_hof ".(strpos($css, "even") !== false ? "even_row" : "odd_row");
			}
			if($view=="today") $timeSince=hoursSince($row['timestamp']);
			if($view=="hof") $timeSince = weeksSince($row['timestamp']);
			$cssw = $view == 'hof' ? "$css weeks_$timeSince" : $css;
?>
			<tr class="<? echo $cssw ?>">
				<td style="text-align:right"><? echo ++$count ?>.</td>
				<td><a class="<? echo $cssw ?>" href="pages/pic.php?id=<?= $row['fileid'] ?>" style="white-space: nowrap; width:312px; overflow:hidden; display:block;"><? echo htmlFilename($row)
			    ?></a></td>
				<td><a class="<? echo $cssw ?>" href="./?c=user&userid=<? echo $row['userid'] ?>"><? echo htmlEscape($row['username']) ?></a></td>
				<td><a class="<? echo $cssw ?>" href="./?c=comments&fileid=<?= $row['fileid'] ?>"><?= $row['comments'] ?></a></td>
				<td>(+<? echo $row['votes'] ?>)</td>
				<td style="text-align:right"><?= $timeSince ?></td>
			</tr>
<?
		}
?>


			</table>


	</div>
<?
	}

	function weeksSince( $timestamp ) {
		return floor( (time() - strtotime( $timestamp )) / (60 * 60 * 24 * 7) );
	}

	function hoursSince( $timestamp) {
		return floor((time() - strtotime($timestamp)) / (60*60));
	}

?>