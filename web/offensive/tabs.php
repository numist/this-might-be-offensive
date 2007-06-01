<? 
	if( ! isset( $activeTab ) ) {
		$activeTab = $_REQUEST['c'];
	}


	function tabs() {
		global $activeTab;
?>


<div>
	<div class="<?= cssFor('images') ?>"><a href="./">images</a></div>
	<div class="<?= cssFor('discussions') ?>"><a href="./?c=discussions">discussions</a></div>
	<div class="<?= cssFor('audio') ?>"><a href="./?c=audio">audio</a></div>	
	<div class="<?= cssFor('hof') ?>"><a href="./?c=hof">hall of fame</a></div>	
	<div class="<?= cssFor('yearbook') ?>"><a href="./?c=yearbook">yearbook</a></div>	
<!-- 
		<div class="<?= cssFor('map') ?>"><a href="./?c=map">map</a></div>
		<div class="<?= cssFor('referral') ?>"><a href="./?c=referral">invite</a></div>	
-->		
		<div class="<?= cssFor('search') ?>"><a href="./?c=search">search</a></div>	
		<div class="<?= cssFor('stats') ?>"><a href="./?c=stats">stats</a></div>	

	<div class="tabspacer">&nbsp;</div>
</div>

<?
	}

	function cssFor( $name ) {
		global $activeTab;
		return $name == $activeTab ? "tabon" : "taboff";
	}
?>
