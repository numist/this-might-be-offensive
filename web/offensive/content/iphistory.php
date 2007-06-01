<?
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'tabs.php' );	
	
	function start() {		
		if( ! ($_SESSION['userid'] == 200 || $_SESSION['userid'] == 151 || $_SESSION['userid'] == 87 )) {
			header( "Location: ./" );
		}

	}

	function body() {
?>
	<div class="heading">spy stuff.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? iphistory( mysql_real_escape_string( $_REQUEST['uid'] ) ); ?>
		</div>
<?
	}

	function iphistory( $uid ) {

?>

<table width="100%">

<?

		$numPerPage = 100;
		$page = $_REQUEST['p'];
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT distinct ip, max(timestamp) as timestamp, userid from ip_history where userid=$uid
			GROUP BY ip
					ORDER BY timestamp DESC LIMIT 100";

		$result = mysql_query($sql);
		echo mysql_error();

?>
				<tr class="<?= $css ?>">
					<td><div class="clipper">ip</div></td>
					<td><div class="clipper">last seen</div></td>
				</tr>
<?

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
					<td><div class="clipper"><a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['ip'] ?></a></div></td>
					<td><div class="clipper"><a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['timestamp'] ?></a></div></td>					
				</tr>
			<?
		}

		
		?>

	</table>


<?
	}
?>
