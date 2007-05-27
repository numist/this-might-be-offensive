<?
	require_once( '../admin/mysqlConnectionInfo.php' );
	require_once( 'tabs.php' );	
	
	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			header( "Location: ./?c=mustLogIn" );
		}
	}

	function body() {
?>
	<div class="heading">moobees!</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? videoList(); ?>
		</div>
<?
	}

	function videoList() {

?>

<table width="100%">

<?

		$numPerPage = 100;
		$page = $_REQUEST['p'];
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT up.*, users.userid, users.username, count( c.id ) AS thecount
					FROM offensive_uploads up, users
					LEFT JOIN offensive_comments c ON fileid = up.id
					WHERE type='video' AND users.userid = up.userid
					GROUP BY up.id
					ORDER BY timestamp DESC
					LIMIT $start, $numPerPage";

		$result = mysql_query( $sql );

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "even_row") ? "odd_row" : "even_row";

			?>
				<tr class="<?= $css ?>">
					<td>
						<div class="clipper">
							<? if( file_exists( $_SERVER['DOCUMENT_ROOT'] . dirname( $_SERVER['PHP_SELF'] ) .  "/images/video/" . $row['filename'] ) ) { ?>
								<a  title="uploaded by <?= $row['username'] ?>" href="./images/video/<?= rawurlencode( $row['filename'] ) ?>"><?= $row['filename'] ?></a>
							<?
							}
							else { 
								echo $row['filename'];
							} ?>
						</div>
					</td>
					<td style="text-align:right;white-space:nowrap"><a href="./?c=comments&fileid=<?= $row['id'] ?>">(<?= $row['thecount'] ?> comments)</a></td>
					<td class="nowrap"><a href="./?c=comments&submit=submit&fileid=<?= $row['id'] ?>&vote=this%20is%20good&redirect=true" title="click to vote [ this is good ] on this file">[ + ]</a></td>
					<td class="nowrap"><a href="./?c=comments&submit=submit&fileid=<?= $row['id'] ?>&vote=this%20is%20bad&redirect=true" title="click to vote [ this is bad ] on this file">[ - ]</a></td>
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
