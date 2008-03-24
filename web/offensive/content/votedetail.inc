<?

	require_once( "offensive/assets/tabs.inc" );

	function start() {
		mustLogIn();
	}

	function body() {
		global $link;

		$uid = $_REQUEST['userid'];

		// Include, and check we've got a connection to the database.
		include_once( 'admin/mysqlConnectionInfo.inc' );
		if(!isset($link) || !$link) $link = openDbConnection();
	
		$sql = "SELECT count( offensive_comments.id ) AS thecount,
						users.username,
						offensive_comments.vote
				FROM users, offensive_comments, offensive_uploads
				WHERE offensive_comments.userid = $uid
					AND offensive_comments.fileid = offensive_uploads.id
					AND offensive_uploads.userid = users.userid
					AND vote
				GROUP  BY users.userid, vote
				ORDER BY username, vote";

		$result = tmbo_query( $sql );
		
		?>
		
<div class="heading">where'd all those votes go? <!-- who do you love? --></div>
<? tabs(); ?>
<div class="bluebox">
<table border="0" width="100%">

	<?
		$good = 0;
		$bad = 0;
		$username = "";
		while( $row = mysql_fetch_assoc( $result ) ) {
			if( $row['username'] != $username && $username != "" ) {
				emitRow( $username, $good, $bad );
				$good = 0;
				$bad = 0;
			}
			
			$username = $row['username'];
			
			switch( $row['vote'] ) {
				case 'this is good':
					$good = $row['thecount'];
				break;
				
				case 'this is bad':
					$bad = $row['thecount'];	
				break;
			}

		}
	?>
	
</table>
</div>
		
		<?

	}

	function emitRow( $name, $good, $bad ) {
	?>
		<tr class="<?= nextStyle() ?>">
			<td><?= $name ?></td>
			<td style="text-align:right">+<?= $good ?></td>
			<td style="text-align:right">-<?= $bad ?></td>
		</tr>
	<?
	}
	
	function nextStyle() {
		global $css;
		$css = ($css == "odd_row") ? "even_row" : "odd_row";
		return $css;
	}

?>