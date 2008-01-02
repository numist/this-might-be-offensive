<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once 'offensive/assets/tabs.inc';
	function body() {
		global $link;

		if(!isset($link) || !$link) $link = openDbConnection();
	
		global $timestamp;
?>

	<div class="heading">numbers and math and stuff.</div>
<?	tabs(); ?>
		<div class="bluebox">

<table width="100%">
	<tr>
		<td>
<?
			stats( "user_uploads", "number of uploads" );
?>
		</td>
		<td>
<?
			upload_quality();
?>
		</td>
		
		<td>
<?
			stats( "employee", "employee of the month" );
?>
		</td>

	</tr>
	<tr>
		<td>
			<?	voteRatio( "ASC", "bad vote % (lowest)" ); ?>
		</td>
		<td>
			<? voteRatio( "DESC", "bad vote % (highest)" ); ?>
		</td>

		<td>
			&nbsp;
		</td>

	</tr>
</table>

<?
		if( isset( $timestamp ) ) {
			echo "<p>Last updated $timestamp</p>";
		}
		
		
		voteCount();
?>


	</div>
<?
	}
	
	
	function voteRatio( $sort_dir, $heading ) {
	
		$sql = "SELECT (b.value / (g.value + b.value)) * 100 AS value, users.username, users.userid
					FROM vote_stats g, vote_stats b, users
					WHERE
					g.userid = users.userid
					AND g.type =  'this is good'
					AND b.type =  'this is bad'
					AND b.value > 0
					AND b.userid = users.userid
					AND (b.value + g.value) > 200
					AND users.timestamp > DATE_SUB( now(), INTERVAL 1 MONTH )
					ORDER  BY value $sort_dir 
					LIMIT 100";

		$result = tmbo_query( $sql );
		statsTable( $result, $heading );
	
	}

	function stats( $vote, $heading ) {
	

		$sql = "SELECT vote_stats.*, users.username FROM vote_stats, users
			WHERE users.userid = vote_stats.userid
			AND type = '$vote'
			ORDER BY value DESC LIMIT 25";

		$result = tmbo_query( $sql );
		statsTable( $result, $heading );
	}
	
	
	function statsTable( $result, $heading ) {

	?>
		<div class="contentbox">
			<div class="heading"><?= $heading ?></div>
			<table width="100%" cellspacing="0" cellpadding="2">
	<?


		$count = 0;
		$style = "even_row";
		while( $row = mysql_fetch_assoc( $result ) ) {
			$style = $style == "even_row" ? "odd_row" : "even_row";
			$count++;
?>
			<tr class="<?= $style ?>">
				<td style="text-align:right"><?= $count ?>.</td>
				<td>
					<?
						if( $row['userid'] == $_SESSION['userid'] ) {
							echo "<b>";
						}
					?>
					<a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?>
					<?
						if( $row['userid'] == $_SESSION['userid'] ) {
							echo "</b>";
						}
					?>
				</td>
				<td style="text-align:right;padding-right:8px;"><?= $row['value'] ?></td>
			</tr>
<?
		}

?>		
		</table>		
	</div>
<?

	}
	
	function voteCount() {
		
		$sql = "SELECT count(id) as thecount
				from offensive_uploads";
		$result = tmbo_query( $sql );
		$row = mysql_fetch_assoc( $result );
		$totalUploads = $row['thecount'];

		$sql = "SELECT count( id ) as thecount
					FROM offensive_uploads
					WHERE timestamp > date_sub( now(), INTERVAL 24 HOUR )";

		$result = tmbo_query( $sql );
		$row = mysql_fetch_assoc( $result );
		$uploadsToday = $row['thecount'];


		$sql = "SELECT vote, count(id) as thecount from offensive_comments group by vote";
		$result = tmbo_query( $sql );
		$total = 0;
		ob_start();
		while( $row = mysql_fetch_assoc( $result ) ) {
			$total += $row['thecount'];
			$vote = $row['vote'];
			$count = $row['thecount'];
			if( $vote != null ) {
				if( isset($not_first_row) && $not_first_row == true) {
					?> and <?
				}
				?><b><?= number_format( $count )?> [<?= $vote?>] votes</b><?
				$not_first_row = true;
			}
		}
		$countstats = ob_get_contents();
		ob_end_clean();		
		?>As of now (<?= date( "F j, Y" ) ?> at <?= date( "g:i") ?>) there have been <b><?= number_format( $total ) ?> comments</b>, including <?= $countstats?>, on <b><?= number_format( $totalUploads )?> uploads</b>, <b><?= $uploadsToday ?></b> of which arrived <b>in the past 24 hours</b>.<?

	}
	
	function upload_quality() {
		
		$sql = "SELECT ( ( good.value - bad.value ) / up.value ) AS value, up.userid, users.username
					FROM vote_stats up, vote_stats good, vote_stats bad, users
					WHERE up.type =  'user_uploads'
						AND good.type =  'good_received'
						AND bad.type =  'bad_received'
						AND up.userid = good.userid
						AND users.userid = up.userid
						AND up.value > 20
					group by up.userid
				ORDER BY value DESC
				LIMIT 25";

		$result = tmbo_query( $sql );
		statsTable( $result, "upload quality" );
	}

?>
