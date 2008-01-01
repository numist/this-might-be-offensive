<?
	require_once( 'offensive/tabs.inc' );

	function start() {
		setcookie( 'thumbnails', "yes", time()+3600*24*365*10, "/offensive/" );
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
#	if( file_exists( 'employeeOfTheMonth.txt' ) ) {
#		include( 'employeeOfTheMonth.txt' );
#	}

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
				<? include( 'offensive/pickupLink.inc' ) ?>
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=main">text view</a></b></div>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
				<?php include 'offensive/indexListThumbnails.txt'; ?>
			</td>
		</tr>
	</table>

</div>

<?
}
?>
