<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	
	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
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
		if( ! is_intger( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		// XXX: really want to use the class system here to reduce code size,
		// but this isn't a workload it's optimized for yet.  if this page ever
		// gets linked from the index, it will need to be optimized to run
		// within the normal dozen-query limit or so.
		$sql = "SELECT *
					FROM users
					ORDER BY created DESC LIMIT $start,100";
//		$sql = "SELECT userid, username, account_status, created,
//					(select username from users ref where users.referred_by=ref.userid) AS referredby,
//					(select userid from users ref where users.referred_by=ref.userid) AS referrerid					
//					FROM users
//					ORDER BY created DESC LIMIT 100";
					
		$result = tmbo_query($sql);

?>
				<tr style="background:#bbbbee;">
					<td><div class="clipper">newb</div></td>
					<td>joined</td>
					<td style="text-align:right;white-space:nowrap">thanks to</td>
				</tr>
<?

		while( $row = mysql_fetch_assoc( $result ) ) {
			$newb = new User($row);
			$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
				<td><div class="clipper"><?= $newb->htmlUsername() ?></div></td>
					<td><?= $newb->created() ?></td>
					<td style="text-align:right;white-space:nowrap"><?= $newb->referred_by()->htmlUsername() ?></td>
				</tr>
			<?
		}

		
		?>

	</table>


<?
	}
?>
