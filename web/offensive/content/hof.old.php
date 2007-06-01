<?

	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once('tabs.php');
	
	function body() {
	
?>

		<div class="heading">hall of fame: the coolest files of all time (according to you sick puppies).</div>
		<? tabs(); ?>
		<div class="bluebox">

			<table width="100%">

			<tr class="<? echo $css ?>">
				<td style="text-align:right"><b>rank</b></td>
				<td><b>filename</b></td>
				<td><b>uploaded by</b></td>
				<td><b>comments</b></td>
				<td><b>votes</b></td>
				<td><b>weeks</b></td>
			</tr>

<?	
		$link = openDbConnection();
		
		$view = isset( $_REQUEST['t'] ) ? mysql_real_escape_string( $_REQUEST['t'] ) : 'hof';
	
		$sql = "SELECT hall_of_fame.fileid, hall_of_fame.votes, up.userid, up.filename, up.timestamp, users.username, up.nsfw
				FROM hall_of_fame, offensive_uploads up, users
				WHERE up.id = fileid AND users.userid = up.userid
				AND hall_of_fame.type = '$view'
				ORDER BY hall_of_fame.id";

		$result = mysql_query( $sql );
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = $css == "evenfile" ? "oddfile" : "evenfile";
			if( $row['userid'] == $_SESSION['userid'] ) {
				$css = "odd_row";
			}
			$weeks = weeksSince( $row['timestamp'] );
?>
			<tr class="<? echo $css ?>">
				<td style="text-align:right"><? echo ++$count ?>.</td>
				<td><a class="<? echo $css ?>" href="images/halloffame/<? echo $row['filename'] ?>"><? echo truncate( (($row['nsfw'] == 1) ? "[nsfw] " : "" ) . $row['filename'], 50 ) ?></a></td>
				<td><a class="<? echo $css ?>" href="./?c=user&userid=<? echo $row['userid'] ?>"><? echo $row['username'] ?></a></td>
				<td><a class="<? echo $css ?>" href="./?c=comments&fileid=<?= $row['fileid'] ?>">comments</a></td>
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
