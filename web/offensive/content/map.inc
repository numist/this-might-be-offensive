<?

	require_once( 'offensive/assets/tabs.inc' );

	function start() {
	
		$usrid = $_SESSION['userid'];
		if( ! is_numeric( $usrid ) ) {
			session_unset();
			header( "Location: ./?c=mustLogIn" );
		}
	
	}

	function head() {
?>
		<script type="text/javascript">
			function showLocation( xloc, yloc ) {
				var x = xloc/10;
				var y = yloc/10;

				// offset for the size of the marker image
				x -= 6;
				y -= 6;

				var map = document.getElementById( "map" );

				if( map.getBoundingClientRect ) {
					var bounds = map.getBoundingClientRect();
					x += bounds.left;
					y += bounds.top;
				}

				x += map.offsetLeft != 0 ? map.offsetLeft : document.documentElement.scrollLeft;
				y += map.offsetTop != 0 ? map.offsetTop : document.documentElement.scrollTop;

				var marker = document.getElementById( "marker" );
				marker.style.left = x + "px";
				marker.style.top = y + "px";				
			}
		</script>
<?
	}

	function body() {
		global $link;
	
	
?>
		<div class="heading">we are duh world.</div>
		<? tabs(); ?>
		<div class="bluebox">
			<div>click the map to see the big picture or to set your location.</div>
			<div style="padding:0px;margin-left:auto;margin-right:auto;width:500px">
				<img src="map/marker.gif" id="marker" style="position:absolute"/>
				<a href="map/"><img src="map/maxxer_map_tiny.jpg" id="map" width="500" height="300" style="margin:0px;"/></a>
			</div>


			<div style="padding:0px;margin-left:auto;margin-right:auto;width:500px;height:250px;overflow:auto">
<table border="0" cellpadding="2" cellspacing="0" width="100%">

<?
	
		$usrid = $_SESSION['userid'];

		// Include, and check we've got a connection to the database.
		include_once( 'admin/mysqlConnectionInfo.inc' );
		if(!isset($link) || !$link) $link = openDbConnection();
		
		$sql = "select *, count( maxxer_locations.id ) from maxxer_locations, users
				where users.userid=maxxer_locations.userid
				group by users.userid
				order by username";

		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		$count = 0;
		$numPerRow = 4;
		while( $row = mysql_fetch_assoc( $result ) ) {
			if( ($count++ % $numPerRow) == 0 ) {
				?><tr><?
			}
?>
			<td><a href="./?c=user&userid=<?= $row['userid'] ?>" onmouseover="showLocation(<?= $row['x'] ?>,<?= $row['y'] ?>)"><?= $row['username'] ?></a></td>
<?		
			if( ($count % $numPerRow) == 0 ) {
				?></tr><?
			}		

		}
?>


</table>
			</div>
		</div>
<?
	
	}


?>
