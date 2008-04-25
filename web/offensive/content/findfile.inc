<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );

	function start() {
		mustLogIn();
	}

	function body() {
		global $link;

// XXX: this should be paginated

// XXX: _REQUEST['since'] gives a lower timestamp bound -- I forget why I wanted this...

		$numperpage = 100;
		$start = (isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			$_REQUEST['p'] : 0) * $numperpage;
		$find = $_REQUEST['findfile'];
	
		$sql = "SELECT offensive_uploads.id, filename, offensive_uploads.timestamp,
					offensive_uploads.type, offensive_uploads.nsfw as is_nsfw,
					offensive_uploads.tmbo as is_tmbo,
					users.userid, username,
					offensive_count_cache.good, offensive_count_cache.bad,
					offensive_count_cache.tmbo, offensive_count_cache.comments
					FROM offensive_uploads, users, offensive_count_cache
					WHERE filename LIKE '%".sqlEscape($find)."%'
					AND offensive_uploads.userid = users.userid
					AND offensive_count_cache.threadid = offensive_uploads.id
					ORDER BY offensive_uploads.timestamp DESC
					LIMIT $start, $numperpage;
				";

		if(!isset($link) || !$link) $link = openDbConnection();
		$result = tmbo_query($sql, 5);
		
		
?>

	<div class="heading">search results</div>
	<div class="bluebox">
	
	
	
<table width="100%">


<?
		if( mysql_num_rows( $result ) == 0 ) {
			echo "<div class=\"piletitle\">No dice, but thanks for playing.</div>";
		}

		while( $row = mysql_fetch_assoc( $result ) ) {
		  
		  $row['filename'] = htmlEscape($row['filename']);
			
			$style = (isset($style) && $style == "") ? "background:#bbbbee;" : "";
			$css = (isset($css) && $css == "evenfile") ? "oddfile" : "evenfile";
			$type = $row['type'];
			$expired = $type == 'image' && getFile($row['id'], $row['filename'], $row['timestamp']) == '' ? "(expired)" : ""
?>
	
			<tr class="<? echo $css ?>" style="<?= $style ?>">
				<? if( $type == 'image' ) { ?>
					<td width="100px"><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $row['id'] ?>"><?

					if(hideImage($row['is_nsfw'], $row['is_tmbo'], $row['userid'])) {
						?>[ filtered ]<?
					} else {
						?><img src="<?= getThumbURL($row['id'], $row['filename'], $row['timestamp']) ?>"/><?
					}
					?></a></td>
					<td><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $row['id'] ?>"><? echo $row['filename'] ?></a> <? echo  $expired ?></td>
					<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?= 
						($row['comments'] != '')?$row['comments']:0;
					?> comments</a> <br />(+<?=
						($row['good'] != '')?$row['good']:0;
					?> -<?=
						($row['bad'] != '')?$row['bad']:0;
					?> x<?=
						($row['tmbo'] != '')?$row['tmbo']:0;
					?>)<br /><?=
						date( "Y-m-d", strtotime( $row['timestamp'] ) )
					?></td>
				<? } ?>
				<? if( $type == 'topic' ) { ?>
					<td width="100px"><? echo date( "Y-m-d", strtotime( $row['timestamp'] ) ) ?></td>
					<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?= $type ?>: <? echo $row['filename'] ?></a></td>
					<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?=
						commentLabel($row['comments'], 0, 0, 0, false);
						?></td>
				<? } ?>
			</tr>
	
<?	
		}
?>


</table>



	</div>

<?
		
	}

?>