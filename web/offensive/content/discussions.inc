<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );
	
	function start() {
		mustLogIn();
	}

	function body() {
?>
	<div class="heading">we need to talk.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<div style="font-weight:bold;text-align:right">
				<a href="./?c=newtopic">new topic</a>
			</div>
			<? discussionList(); ?>
		</div>
<?
	}

	function discussionList() {

?>

<table width="100%">

	<tr style="background:#bbbbee;text-align:center;">
		<td><b><a href="./?c=discussions&order=up.timestamp">sort by thread creation date</a></b></td>
		<td><b><a href="./?c=discussions&order=thecount">comment count</a></b></td>
		<td><b><a href="./?c=discussions&order=latest">latest comment</a></b></td>
	</tr>

<?

		$defaultSort = isset( $_SESSION['prefs']['sortorder'] ) ? $_SESSION['prefs']['sortorder'] : "up.timestamp";

		$order = isset($_REQUEST['order']) ? sqlEscape( $_REQUEST['order'] ) : $defaultSort;

		if( $order != $defaultSort ) {
			// should make this persistent
			$_SESSION['prefs']['sortorder'] = $order;
		}

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "select up.*, up.id as fileid, counts.*, counts.timestamp AS latest, counts.comments AS thecount
				FROM offensive_uploads up
				LEFT JOIN offensive_count_cache counts ON counts.threadid = up.id
				WHERE type = 'topic'
					ORDER BY $order DESC
					LIMIT $start, $numPerPage";

		$result = tmbo_query( $sql );

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
			
			$filename = trim($row['filename']);
			$filename = htmlEscape($filename);

			?>
				<tr class="<?= $css ?>">
					<td><a href="./?c=comments&fileid=<?= $row['fileid'] ?>"><?= $filename ?></a></td>
					<td style="text-align:right"><a href="./?c=comments&fileid=<?= $row['fileid'] ?>">(<?= $row['comments'] ?> comments)</a></td>
					<td style="text-align:right"><?= $row['latest'] ?></td>
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
				<td></td>
				<td style="text-align:right">
					<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
						<a href="<?= $_SERVER['PHP_SELF']?>?c=<?= $_REQUEST['c'] ?>&order=<?=$order?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;</td>
					<? } ?>
			</tr>

	</table>


<?
	}
?>