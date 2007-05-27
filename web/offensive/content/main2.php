<?

	require_once( 'tabs.php' );	
	
	function start() {
		setcookie( 'thumbnails', "no", time()-3600, "/offensive/" );
	}

	function body() {
		
		if( file_exists( "./updating.txt" ) ) {
			$updating_now = true;
		}
		
		if( file_exists( "./log.txt" ) ) {
			$last_update = filemtime( "log.txt" );
		} else {
			$last_update = -1;
		}

?>
<div class="heading">

<?
	if( file_exists( 'employeeOfTheMonth.txt' ) ) {
		include( 'employeeOfTheMonth.txt' );
	}

	?>

</div>

<?
	global $activeTab;
	$activeTab = "images";
	tabs();
?>

<div class="bluebox">
		
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td valign="top">
				<?
					$lastpic = $_COOKIE["lastpic"];
					if( $lastpic ) {
						echo "<div><b><a href=\"pages/pic.php?id=$lastpic\">click here to pick up where you left off</a></b></div>";
					}
				?>			
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=thumbs">thumbnail view</a></b></div>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
				<?php include 'indexList.txt'; ?>
			</td>
		</tr>
	</table>
			
</div>

<?
}
?>
