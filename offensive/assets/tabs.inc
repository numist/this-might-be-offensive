<? 
	if( ! isset( $activeTab ) ) {
		$activeTab = array_key_exists("c", $_REQUEST) ? $_REQUEST['c'] : "images";
	}


	function tabs() {
		global $activeTab;
?>


<div>
	<div class="<?= cssFor('images') ?>"><a href="<?= Link::mainpage() ?>">images</a></div>
	<div class="<?= cssFor('discussions') ?>"><a href="<?= Link::content("discussions") ?>">discussions</a></div>
	<div class="<?= cssFor('hof') ?>"><a href="<?= Link::content("hof") ?>">hall of fame</a></div>	
	<div class="<?= cssFor('audio') ?>"><a href="<?= Link::content("audio") ?>">audio</a></div>	
	<div class="<?= cssFor('yearbook') ?>"><a href="<?= Link::content("yearbook") ?>">yearbook</a></div>	
	<div class="<?= cssFor('search') ?>"><a href="<?= Link::content("search") ?>">search</a></div>	

	<div class="tabspacer">&nbsp;</div>
</div>

<?
	}

	function cssFor( $name ) {
		global $activeTab;
		return $name == $activeTab ? "tabon" : "taboff";
	}
?>