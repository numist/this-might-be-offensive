<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/tabs.inc' );
	
	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			header( "Location: ./" );
		}
	}

	function body() {
?>
	<div class="heading">newbies.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? newbs(); ?>
		</div>
<?
	}

	function newbs() {

?>

<table width="100%">

<?

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT userid, username, account_status, created,
					(select username from users ref where users.referred_by=ref.userid) AS referredby,
					(select userid from users ref where users.referred_by=ref.userid) AS referrerid					
					FROM users
					ORDER BY created DESC LIMIT 100";
					
		$result = mysql_query($sql) or trigger_error(mysql_error(), E_USER_ERROR);

?>
				<tr style="background:#bbbbee;">
					<td><div class="clipper">newb</div></td>
					<td>joined</td>
					<td style="text-align:right;white-space:nowrap">thanks to</td>
				</tr>
<?

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
					<td><div class="clipper"><a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?></a></div></td>
					<td><?= $row['created']?></td>
					<td style="text-align:right;white-space:nowrap"><a href="./?c=user&userid=<?= $row['referrerid'] ?>"><?= $row['referredby'] ?></a></td>
				</tr>
			<?
		}

		
		?>

	</table>


<?
	}
?>
