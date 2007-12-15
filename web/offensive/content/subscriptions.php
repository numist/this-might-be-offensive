<?php
	session_start();
	
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'functions.php' );
	require_once( 'getPrefs.php' );
	require_once( 'tabs.php' );	
		
	function title() {
		global $filename;
		return "[ tmbo ] : subscriptions";
	}

	function start() {
		global $filename;
		
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./?c=mustLogIn" );
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
			
			


		$result = mysql_query( $sql );
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
		  $row['filename'] = str_replace(array("<", ">", "\""), array("&lt;", "&gt;", "&quot;"),
					           preg_replace("/&(?!#)/", "&amp;", $row['filename'] ));
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
