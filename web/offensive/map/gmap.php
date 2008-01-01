<?
	set_include_path("../..");
	require_once( 'offensive/assets/header.inc' );

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../" );
	}

	require_once( 'admin/mysqlConnectionInfo.inc' );

	$x = $_REQUEST['x'];
	$y = $_REQUEST['y'];

	if( is_numeric( $x ) ) {
		setMaxxerLoc( $_SESSION['userid'], $x, $y );
		header( "Location: ./?p=1" );
		exit;
	}

	function setMaxxerLoc( $maxxerid, $x, $y ) {
		global $link;

		if( ! is_numeric( $maxxerid ) ) {
			return;
		}

		if(!isset($link) || !$link) $link = openDbConnection();
		$sql = "replace into maxxer_locations (userid, x, y, mapversion) values ( $maxxerid, $x, $y, 'google' )";
		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		
	}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
  
<html>
<head>
	<title></title>
		<style type="text/css">
		body {
			font-family:verdana;
			font-size:11px;
			color:#333366;
		}
		
		a {
			color:#333366;
		}
		
		.copyright {
			font-size:10px;
		}
	</style>
		
	<!--
	for tmbo.org
    <script src="http://maps.google.com/maps?file=api&amp;v=2&amp;key=ABQIAAAAiLrNiTQTZlY9-a3cY0h3vBRk1r88Hrq2n74rpCHBF8iHJqU5OBRFTMsCD4pR4wEehvSKSO3kXYsg8g" type="text/javascript"></script>
	-->
    <script src="http://maps.google.com/maps?file=api&amp;v=2&amp;key=ABQIAAAAyBBXeWMcfhIUkB7pKzhc9xQjSqRAlqXjnjIvJJ9RLUDSqpvsZRTIaaZHx9vj_4c_8qVi_4wR2bZzqg" type="text/javascript"></script>
	<script type="text/javascript" src="/bitsUtils.js"></script>
	<script type="text/javascript">
	
	var icon;
	var maxxers = {};
	
	 function load() {
      if (GBrowserIsCompatible()) {
        var map = new GMap2(document.getElementById("map"));
        map.setCenter(new GLatLng(35.55, -97.554101), 1);
        map.addControl(new GLargeMapControl());
		map.addControl(new GMapTypeControl());
		
		// Create our "tiny" marker icon
		icon = new GIcon();
		icon.image = "maxxermarker.png";
		icon.shadow = "maxxermarkershadow.png";
		icon.iconSize = new GSize(12, 20);
		icon.shadowSize = new GSize(22, 20);
		icon.iconAnchor = new GPoint(6, 20);
		icon.infoWindowAnchor = new GPoint(5, 1);
		
		<?
			$link = openDbConnection();
			$sql = "select *, username from maxxer_locations, users where mapversion = 'google' AND maxxer_locations.userid = users.userid";
			$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
			
			while( $row = mysql_fetch_array( $result ) ) {
				?>
					maxxers[ "<?= $row['y'] ?>,<?= $row['x'] ?>" ] = "<?= $row['username'] ?>";
					map.addOverlay( new GMarker( new GLatLng( <?= $row['y'] ?>, <?= $row['x'] ?> ), icon ) );<?
			}

		?>

		GEvent.addListener(map, "click", function( marker, point ) {
				if( marker != null ) {
					var name = maxxers[ marker.getPoint().y + "," + marker.getPoint().x ];
					if( name != null ) {
						marker.openInfoWindow( name );
					}
				}
				else {
					setLocation( point.x, point.y );
					map.clearOverlays();
				 	map.addOverlay(new GMarker( point ), icon);
				 }
			}
		);
		
	}
	}
	
	function setLocation( x, y ) {
		makeRequest( "./gmap.php?x=" + x + "&y=" + y, function( req ) {
			if( req.status == 200 ) {

			}
		});
	}
	
	</script>
</head>

  <body onload="load()" onunload="GUnload()">
	<div style="margin:18px;">
		<p>Click the map to set your location.</p>
		<p><a href="./">Click here</a> for the old map.</p>
		<p><a href="../">Return to [ tmbo ]</a></p>
	</div>
	<div id="map" style="width: 800px; height: 360px"></div>
  </body>

</html>

