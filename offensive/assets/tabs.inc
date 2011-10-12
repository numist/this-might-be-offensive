<? 
	if( ! isset( $activeTab ) ) {
		$activeTab = array_key_exists("c", $_REQUEST) ? $_REQUEST['c'] : "images";
	}


	function tabs() {
		global $activeTab;
?>


<div>
	<div class="<?= cssFor('images') ?>"><a href="/offensive/<? 
		if(me()) {
			echo "?c=".((me()->getPref("index") == "thumbs") ? 
	      "thumbs" : "main"); 
		}
	?>">images</a></div>
	<div class="<?= cssFor('discussions') ?>"><a href="/offensive/?c=discussions">discussions</a></div>
	<div class="<?= cssFor('hof') ?>"><a href="/offensive/?c=hof">hall of fame</a></div>	
	<div class="<?= cssFor('audio') ?>"><a href="/offensive/?c=audio">audio</a></div>	
	<div class="<?= cssFor('yearbook') ?>"><a href="/offensive/?c=yearbook">yearbook</a></div>	
	<div class="<?= cssFor('search') ?>"><a href="/offensive/?c=search">search</a></div>	

	<div class="tabspacer">&nbsp;</div>
</div>

<?
	}

	function cssFor( $name ) {
		global $activeTab;
		return $name == $activeTab ? "tabon" : "taboff";
	}
?>