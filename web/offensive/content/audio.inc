<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	
	function start() {
		mustLogIn();
	}

	function body() {
?>
	<div class="heading">listen up.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? audioList(); ?>
		</div>
<?
	}

	function thisOrZero( $value ) {
		return (is_numeric( $value ) ? $value : 0);
	}

	function audioList() {

?>

<table width="100%">

<?

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "select up.*, up.id as fileid, counts.*, counts.timestamp AS latest, counts.comments AS thecount
				FROM offensive_uploads up
				LEFT JOIN offensive_count_cache counts ON counts.threadid = up.id
				WHERE type = 'audio'
					ORDER BY up.timestamp DESC
					LIMIT $start, $numPerPage";


		$result = tmbo_query( $sql );

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
					<td>
						<div class="clipper">
							<? if( file_exists( $_SERVER['DOCUMENT_ROOT'] . dirname( $_SERVER['PHP_SELF'] ) .  "/images/audio/" . $row['filename'] ) ) { ?>
								<a  title="uploaded by <?= $row['username'] ?>" href="./images/audio/<?= rawurlencode( $row['filename'] ) ?>"><?= $row['filename'] ?></a>
							<?
							}
							else { 
								echo $row['filename'];
							} ?>
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
