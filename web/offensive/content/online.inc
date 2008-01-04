<?	
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/functions.inc' );	
	require_once( 'offensive/assets/tabs.inc' );

	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			session_unset();
			header( "Location: ./" );
		}
	}
	

	function body() {
		global $timelimit;

		// get the total number of users online
		$sql = "SELECT COUNT(*) FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE)";
		$result = tmbo_query($sql);
		list($nonline) = mysql_fetch_array($result);

?>

	<div class="heading"><?= $nonline ?> folks online in the last <?= $timelimit ?> minutes.</div>
	<? tabs(); ?>
	<div class="bluebox">

<table width="100%">

<!--	<tr style="background:#bbbbee;text-align:center;">
		<td><b><a href="./?c=discussions&order=up.timestamp">sort by thread creation date</a></b></td>
		<td><b><a href="./?c=discussions&order=thecount">comment count</a></b></td>
		<td><b><a href="./?c=discussions&order=latest">latest comment</a></b></td>
	</tr> -->
	<tr class="even_row">
		<td>you</td>
		<td style="text-align:right">now</td>
	</tr>
	<?
		$css = "even";
		$sql = "SELECT userid, username, timestamp FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE) && userid != ".$_SESSION['userid']." ORDER BY timestamp DESC";
		$result = tmbo_query( $sql );
		while(false !== ($row = mysql_fetch_assoc( $result ) )) {
			$css = $css == "odd" ? "even" : "odd"; ?>
				<tr class="<?= $css ?>_row">
					<td><a href="./?c=user&userid=<?= $row['userid']?>"><?= $row['username'] ?></a></td>
					<td style="text-align:right"><?
		$timeago = time() - strtotime($row['timestamp']);
		$minago = (int)($timeago / 60);
		$secago = $timeago % 60;
		if($minago > 0) {
			echo "$minago:";
			if(strlen($secago) == 1) echo "0";
			echo "$secago ago";
		} else if ($secago == 0) {
			echo "now";
		} else echo $secago."s ago"; ?></td>
				</tr>
	<?	} ?>

	</table>

	</div>

<?
		
	}

?>
