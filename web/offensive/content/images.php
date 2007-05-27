<?
	header( "Location: ./?c=main" );
	exit;

	require_once( '../admin/mysqlConnectionInfo.php' );
	require_once( 'tabs.php' );	
	
	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			header( "Location: ./?c=mustLogIn" );
		}
	}

	function body() {
?>
	<div class="heading">lookie!</div>
<?

	global $activeTab;
	$activeTab = "images";
	tabs();

?>
		<div class="bluebox">
			<? itemList( 'image' ); ?>
		</div>
<?
	}

	function thisOrZero( $value ) {
		return (is_numeric( $value ) ? $value : 0);
	}

	function itemList( $type ) {

?>

<table width="100%">
		<tr>
			<td valign="top">
				<? include( 'pickupLink.php' ) ?>
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=thumbs">thumbnail view</a></b></div>
			</td>
		</tr>

<?

		$numPerPage = 100;
		$page = $_REQUEST['p'];
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "select up.*, up.id as fileid, counts.*, counts.timestamp AS latest, counts.comments AS thecount
				FROM offensive_uploads up
				LEFT JOIN offensive_count_cache counts ON counts.threadid = up.id
				WHERE type = '$type'
					ORDER BY up.timestamp DESC
					LIMIT $start, $numPerPage";


		$result = mysql_query( $sql );

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
					<td>
						<div class="clipper">
							<a title="uploaded by <?= $row['username'] ?>" href="./pages/pic.php?id=<?= $row['fileid'] ?>"><?= $row['filename'] ?></a>
						</div>
					</td>
					<td style="text-align:right;white-space:nowrap"><a href="./?c=comments&fileid=<?= $row['fileid'] ?>"><?= thisOrZero( $row['comments'] ) ?> comments</a> (+<?= thisOrZero( $row['good'] )?> -<?= thisOrZero( $row['bad'] )?>)</td>
				</tr>
			<?
		}

		
		?>
			<tr>
				<td>
					<? if( $page > 0 ) { ?>
						<a href="<?= $_SERVER['PHP_SELF']?>?c=<?= $_REQUEST['c'] ?>&order=<?=$order?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
					<? } ?>
				</td>
				<td style="text-align:right">
					<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
						<a href="<?= $_SERVER['PHP_SELF']?>?c=<?= $_REQUEST['c'] ?>&order=<?=$order?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
					<? } ?>
				</td>
			</tr>

	</table>


<?
	}
?>
