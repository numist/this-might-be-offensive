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
				<td><b>uploaded by</b></td>
				<td><b>comments</b></td>
				<td><b>votes</b></td>
				<td><b>weeks</b></td>
			</tr>

<?	
		$sql = "SELECT hall_of_fame.fileid, hall_of_fame.votes, up.userid, up.filename, up.timestamp, users.username, up.nsfw
				FROM hall_of_fame, offensive_uploads up, users
				WHERE up.id = fileid AND users.userid = up.userid
				AND hall_of_fame.type = '$view'
				ORDER BY hall_of_fame.id";

		$result = tmbo_query( $sql );
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = $css == "evenfile" ? "oddfile" : "evenfile";
			if( $row['userid'] == $_SESSION['userid'] ) {
				$css = "odd_row";
			}
			$weeks = weeksSince( $row['timestamp'] );
			$cssw = $view == 'hof' ? "$css weeks_$weeks" : $css;
?>
			<tr class="<? echo $cssw ?>">
				<td style="text-align:right"><? echo ++$count ?>.</td>
				<td><a class="<? echo $cssw ?>" href="images/halloffame/<? echo $row['filename'] ?>"><? echo truncate( (($row['nsfw'] == 1) ? "[nsfw] " : "" ) . htmlEscape($row['filename']), 50 )
			    ?></a></td>
				<td><a class="<? echo $cssw ?>" href="./?c=user&userid=<? echo $row['userid'] ?>"><? echo htmlEscape($row['username']) ?></a></td>
				<td><a class="<? echo $cssw ?>" href="./?c=comments&fileid=<?= $row['fileid'] ?>">comments</a></td>
				<td>(+<? echo $row['votes'] ?>)</td>
				<td style="text-align:right"><?= $weeks ?></td>
			</tr>
<?
		}
?>


			</table>


	</div>
<?
	}

	function truncate( $input, $maxlength ) {
	
		if( strlen( $input ) <= $maxlength ) {
			return $input;
		}
		
		return substr( $input, 0, $maxlength ) . "...";

	}

	function weeksSince( $timestamp ) {
		return floor( (time() - strtotime( $timestamp )) / (60 * 60 * 24 * 7) );
	}

?>