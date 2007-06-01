<?
		ob_start();
		session_start();

		header('Content-type: text/xml');

		// Include, and check we've got a connection to the database.
		include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

		$uid = $_SESSION['userid'];

		if( ! is_numeric( $uid ) ) {
			return;
		}

		$sql = "SELECT fileid, filename, min(commentid) as commentid
					FROM offensive_uploads u, offensive_bookmarks b
					WHERE b.userid = $uid AND u.id = b.fileid
					group by fileid
					LIMIT 50";

		$link = openDbConnection();

		$result = mysql_query( $sql );
		
		if( mysql_num_rows( $result ) == 0 ) {
			?><div>none</div><?
			return;
		}
	
	?>
	
	
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">unread comments:</div>
					<div class="bluebox">
	<?
	
		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = $css == "evenfile" ? "oddfile" : "evenfile";
	?>
			<div class="clipper"><a class="<?= $css ?>" href="./?c=comments&amp;fileid=<?= $row['fileid'] ?>#<?= $row['commentid']?>"><?= htmlspecialchars( $row['filename'] ) ?></a></div>
	<?
	
		}
	?>

					</div>

					<div class="heading" style="text-align:center">
						<a class="orange" href="markallread.php">mark all read</a>
					</div>

				<div class="blackbar"></div>
			</div>
	

	<?
?>
