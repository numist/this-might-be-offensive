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
	<div class="heading">the beautiful people.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? yearbook(); ?>
		</div>
<?
	}

	function yearbook() {

		$order = array_key_exists("sort", $_REQUEST) && $_REQUEST['sort'] == 'date' ? 
		         'up.timestamp DESC' :
			 'username';

		if( $order == 'username' ) {
			?><b><a href="./?c=yearbook&sort=date">sort by date</a></b><?
		} else {
			?><b><a href="./?c=yearbook">sort by username</a></b><?
		}
?>



<table width="100%" class="thumbnails">

<?

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;

		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT users.userid, username, account_status, filename, up.id, up.timestamp
					FROM users, offensive_uploads up
					WHERE users.userid=up.userid
						AND up.type='avatar'
						AND account_status != 'locked'
						AND status != 'pending'
						AND users.timestamp > DATE_SUB( NOW(), INTERVAL 12 MONTH )
						AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up WHERE type='avatar' AND userid=users.userid)
					ORDER BY $order, up.id DESC";

		$result = tmbo_query($sql);

		$THUMBS_PER_ROW = 4;
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
			if(getThumb($row['id'], $row['filename'], $row['timestamp'], 'avatar') == '') continue;
		
			if( (($count++) % $THUMBS_PER_ROW) == 0 ) {
				$css = (isset($css) && $css == "even_row") ? "odd_row" : "even_row";
				?><tr class="<?= $css ?>"><?
			}
		
			?>
				
					<td>
						<a href="<?= getFileURL($row['id'], $row['filename'], $row['timestamp'], 'avatar'); ?>"><img src="<?=
							getThumbURL($row['id'], $row['filename'], $row['timestamp'], 'avatar');
						?>"/></a>
						<br/>
						<a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?></a>
						<a href="./?c=comments&fileid=<?= $row['id'] ?>">[c]</a>						
					</td>

			<?
			
			if( (($count) % $THUMBS_PER_ROW) == 0 ) {
				?></tr><?
			}
		}

		if( ($count % $THUMBS_PER_ROW) != 0 ) {
			?></tr><?
		}
		?>

	</table>


<?
	}
?>