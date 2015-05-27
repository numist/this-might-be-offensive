<?
		set_include_path("..");
		require_once("offensive/assets/header.inc");
		// Include, and check we've got a connection to the database.
		include_once( 'admin/mysqlConnectionInfo.inc' );
		if(!isset($link) || !$link) $link = openDbConnection();

		mustLogIn();

		header('Content-type: text/xml');

		$uid = $_SESSION['userid'];

		$sql = "SELECT DISTINCT u.*, b.commentid
					FROM offensive_uploads u, offensive_subscriptions b
					WHERE b.userid = $uid 
						AND u.id = b.fileid
						AND b.commentid IS NOT NULL
					GROUP BY u.id
					LIMIT 50";

		$result = tmbo_query( $sql );
		
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
			// XXX: rejigger the query and use Link::comment ?>
			<div class="clipper"><a class="<?= $css ?>" href="<?= Link::thread(new Upload($row)) ?>#<?= $row['commentid']?>"><?= htmlspecialchars( $row['filename'] ) ?></a></div>
	<?
	
		}
	?>

					</div>
				<div class="blackbar"></div>
			</div>
