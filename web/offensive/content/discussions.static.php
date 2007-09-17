<?

	require_once( 'tabs.php' );	

	function body() {
		
?>
<div class="heading">
	we need to talk.
</div>

<?
	global $activeTab;
	$activeTab = "discussions";
	tabs();
?>

<div class="bluebox">
	<div style="font-weight: bold; text-align: right;">
		<a href="./?c=newtopic">new topic</a>
	</div>
	
	<?php include 'discussionsList.txt'; ?>
			
</div>

<?
}
?>