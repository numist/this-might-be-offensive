<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once('offensive/assets/classes.inc');

	function start() {
		mustLogIn();
		
		if(!isset($_REQUEST['findfile']))
			header("Location: ./");
	}

	function body() {
// XXX: _REQUEST['since'] gives a lower timestamp bound -- I forget why I wanted this...

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;
		$find = $_REQUEST['findfile'];
	
		$sql = "SELECT offensive_uploads.*,
					users.username AS user_username,
					offensive_count_cache.good AS goods, offensive_count_cache.bad AS bads,
					offensive_count_cache.tmbo AS tmbos, offensive_count_cache.comments
					FROM offensive_uploads, users, offensive_count_cache
					WHERE offensive_uploads.filename LIKE '%".sqlEscape($find)."%'
					AND offensive_uploads.userid = users.userid
					AND offensive_count_cache.threadid = offensive_uploads.id
					AND offensive_uploads.status != 'pending'
					ORDER BY offensive_uploads.timestamp DESC
					LIMIT $start, $numPerPage;
				";

		$result = tmbo_query($sql, 5);
		
		
?>

	<div class="heading">search results</div>
	<div class="bluebox">
	
	
	
<table width="100%">


<?
		if( mysql_num_rows( $result ) == 0 ) {
			echo "<div class=\"piletitle\">No dice, but thanks for playing.</div>";
		}
		
		$me = new User($_SESSION['userid']);

		while( $row = mysql_fetch_assoc( $result ) ) {
			$style = (isset($style) && $style == "") ? "background:#bbbbee;" : "";
			$css = (isset($css) && $css == "evenfile") ? "oddfile" : "evenfile";
			
			$upload = new Upload($row);
			
			$filename = $upload->htmlFilename($upload->type() != 'image');
			
			$expired = $upload->expired() ? "(expired)" : "";
?>
	
			<tr class="<? echo $css ?>" style="<?= $style ?>">
				<? if( $upload->type() == 'image' || $upload->type() == 'avatar') { ?>
					<td width="100px" height="100px">
					
					
					<a href="pages/pic.php?id=<?= $upload->id() ?>"
						<? if($upload->filtered()) { ?>
							onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
					 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
						<? } ?>
					><img name="th<?= $upload->id()?>"
						src="<?= $upload->filtered()
							? "/offensive/graphics/th-filtered.gif" 
							: $upload->thumbURL() ?>"
					/></a>
					
					
					
					</td>
					<td><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $upload->id() ?>"><?= $filename ?></a> (uploaded by <?= htmlEscape($upload->uploader()->username()) ?>)<? echo  $expired ?></td>
					<td width="170px" align="center"><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $upload->id() ?>"><nobr><?= 
						$upload->commentLabel();
					?></nobr><br /><?=
						date( "Y-m-d", strtotime( $upload->timestamp() ) )
					?></td>
				<? } else if( $upload->type() == 'topic' ) { ?>
					<td width="100px"><? echo date( "Y-m-d", strtotime( $upload->timestamp() ) ) ?></td>
					<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $upload->id() ?>"><?= $filename ?></a></td>
					<td width="100px" align="center"><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $upload->id() ?>"><?=
						$upload->commentLabel(false);
						?></td>
				<? } ?>
			</tr>
	
<?	
		}
?>

	<tr>
		<td>
			<? if( $page > 0 ) { ?>
				<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&findfile=<?= urlencode($_REQUEST['findfile']) ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
			<? } ?>
		</td>
		<td></td>
		<td style="text-align:right">
			<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
				<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&findfile=<?= urlencode($_REQUEST['findfile']) ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
			<? } ?>
		</td>
	</tr>


</table>



	</div>

<?
		
	}

?>