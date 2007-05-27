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

		$order = isset($_REQUEST['order']) ? mysql_real_escape_string( $_REQUEST['order'] ) : $defaultSort;

		if( $order != $defaultSort ) {
			// should make this persistent
			$_SESSION['prefs']['sortorder'] = $order;
		}

		$sql = "SELECT up.*, count( c.id ) AS thecount, max( c.timestamp + 0 ) AS latest
					FROM offensive_uploads up
					LEFT JOIN offensive_comments c ON fileid = up.id
					WHERE type='topic'
					GROUP BY up.id
					ORDER BY $order DESC";
		$result = mysql_query( $sql );
		
		echo "<!--" . mysql_error() . "-->";

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "even_row") ? "odd_row" : "even_row";
			
			$latest = $row['latest'];
			$year = substr( $latest, 0, 4 );
			$month = substr( $latest, 4, 2 );
			$day = substr( $latest, 6, 2 );
			$hour = substr( $latest, 8, 2 );
			$minute = substr( $latest, 10, 2 );
			$second = substr( $latest, 12, 2 );
			$date = strtotime( "$year-$month-$day $hour:$minute:$second" );
			
			?>
				<tr class="<?= $css ?>">
					<td><a class="<?= $css ?>" href="./?c=comments&fileid=<?= $row['id'] ?>"><?= $row['filename'] ?></a></td>
					<td style="text-align:right"><a href="./?c=comments&fileid=<?= $row['id'] ?>">(<?= $row['thecount'] ?> comments)</a></td>
					<td style="text-align:right"><?= date( "Y-m-d h:i:s", $date ) ?></td>
				</tr>
			<?
		}
?>

</table>


<?
	}
?>
