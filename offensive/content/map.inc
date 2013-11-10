<?

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	

	function start()
	{
		mustLogIn();
	}

	function head()
	{ 	?>
		<script src="https://maps.googleapis.com/maps/api/js?v=3.exp&sensor=false" type="text/javascript"></script>
		<?
		CSS::add("/styles/map.css");
		JS::add("/offensive/js/markermanager-1.2.js");
		JS::add("/offensive/js/maps-3.0.js");
		?>
		<?
	}

	function body()
	{
		?>
	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading">
			long time no see. do YOU know where YOUR maxxers are?
		</div><?
		
		global $activeTab;
		$activeTab = "map";
		tabs();
		
		$edit = (isset($_GET['action']) && ($_GET['action'] == 'edit')) ? true : false;
		$userid = me()->id();
?>
		<div id="map" style="width: auto; height: 500px;"></div>
<?php if(!$edit) {?>
		<div id="sidebar" style="padding: 5px; width: auto; height: 250px; overflow: auto;"></div>
<?php } else { ?>
		<div id="sidebar" style="padding: 5px; width: auto; height: 250px; overflow: auto;">
			<form action="#" onsubmit="maxxerMap.showAddress(this.address.value); return false">
				<input type="text" size="35" name="address" onfocus="this.value=''" value="eg: Main Street 1, Smallville, Ohio" />
				<input type="submit" value="Search" />
			</form>
		</div>
<?php } ?>
		<div id="map_edit" style="border: 1px solid black; padding: 0; margin: 0; text-align: center; background-color: #fff; width: 100px; height: 15px;"><b><a href="<?= Link::content("map").($edit ? '">Back' : '&action=edit">Add/Edit') ?></a></b></div>
		<script>
			var mapEdit = <?= $edit ? 'true' : 'false' ?>;
		</script>
		<div class="blackbar"></div>
	</div>
		
<?
	}
?>