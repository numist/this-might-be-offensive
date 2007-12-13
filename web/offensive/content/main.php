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

	<!-- welcome back. we missed you. oh. and don't use apostrophes for plurals. ever. -->
	if you don't understand, upload a picture of yer mom and we'll do a little experiment.

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
				<? include( 'pickupLink.php' ) ?>
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
