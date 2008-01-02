<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/getPrefs.inc' );
	require_once( 'offensive/assets/tabs.inc' );
		
	function title() {
		global $filename;
		return "[ tmbo ] : subscriptions";
	}

	function start() {
		global $filename;
		
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./" );
			exit;
		}
	}

	function body() {
	
	
		$usrid = $_SESSION['userid'];
		$link = openDbConnection();
		$sql = "select distinct s.fileid, u.filename
			from offensive_subscriptions s,
				offensive_uploads u
			where s.userid=" . $usrid . " 
				and s.fileid=u.id
			order by u.timestamp DESC
			limit 600";
			
			


		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		echo mysql_error();

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

	?>	</div><?
	
	}

	function nextStyle() {
		global $style;
		$style = $style == "background:#bbbbee;" ? "" : "background:#bbbbee;";
		return $style;
	}


?>
