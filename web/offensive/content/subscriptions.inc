<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );
		
	function title() {
		global $filename;
		return "[ tmbo ] : subscriptions";
	}

	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
	}

	function body() {
		$usrid = $_SESSION['userid'];

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;

		$sql = "SELECT DISTINCT s.fileid, u.filename
			FROM offensive_subscriptions s,
				offensive_uploads u
			WHERE s.userid=$usrid 
				AND s.fileid=u.id
			ORDER BY s.fileid DESC
			LIMIT $start, $numPerPage";

		$result = tmbo_query( $sql );
?>	


		<div class="heading">
			subscriptions.
		</div>
	
<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div class="bluebox" style="text-align:left">	
<?
		while( $row = mysql_fetch_assoc( $result ) ) {
		  $row['filename'] = htmlEscape($row['filename']);
			?>
		
			<div class="entry" style="<?php echo nextStyle()?>">
				<a href="subscribe.php?un=1&fileid=<?= $row['fileid'] ?>" style="float:right" title="take this thread off my 'unread comments' watch list.">unsubscribe</a>
				<a href="./?c=comments&fileid=<?= $row['fileid'] ?>"><?= $row['filename'] ?></a>				
			</div>
		
			<? 		
		}

	?>
		<table width="100%">
			<tr>
				<td>
					<? if( $page > 0 ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
					<? } ?>
				</td>
				<td style="text-align:right">
					<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
					<? } ?>
				</td>
			</tr>        
		</table>
		</div>
	<?
	}

	function nextStyle() {
		global $style;
		$style = $style == "background:#bbbbee;" ? "" : "background:#bbbbee;";
		return $style;
	}


?>