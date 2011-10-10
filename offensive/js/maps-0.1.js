var maxxerIcon = new GIcon();
maxxerIcon.image = "/offensive/graphics/map_man_blue.png";
maxxerIcon.shadow = "/offensive/graphics/map_man_shadow.png";
maxxerIcon.iconSize = new GSize(23, 23);
maxxerIcon.shadowSize = new GSize(40, 23 );
maxxerIcon.iconAnchor = new GPoint(12, 23);
maxxerIcon.infoWindowAnchor = new GPoint(14, 6);
maxxerIcon.infoShadowAnchor = new GPoint(20, 23);

var clusterIcon = new GIcon(maxxerIcon);
clusterIcon.image = "/offensive/graphics/map_cluster.png";
clusterIcon.iconSize = new GSize(27, 27);
clusterIcon.shadowSize = new GSize(48, 27 );
clusterIcon.iconAnchor = new GPoint(12, 25);


var map;
var geocoder;
var sidebar_html = "";
var gmarkers = [];
var mgr;
var find_id = -1;
var click_id;

// this function is called when a user clicks on an icon
function myclick(i)
{
	map.setCenter(gmarkers[i].getPoint(), 8);
	// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
	GDownloadUrl("/offensive/map/get_userinfo.php?user=" + gmarkers[i].id, function(data) {
		 map.openInfoWindow(gmarkers[i].getLatLng(), data, {pixelOffset: new GSize(5,-20)});
	});
}

// if we click on a posse link in the infowindow we need to find the right
// marker. Once we find it, force a click
function posse_click(i)
{
	for(var j=0; j<gmarkers.length; j++) {
		if(gmarkers[j].id == i) {
			myclick(j);
			break;
		}
	}
}

// this starts the map
function loadmap()
{
	if (GBrowserIsCompatible()) {
		var i = 0;

		// this function creates a marker on the screen and assembles
		// the sidebar HTML
		function createMarker(point, name, userid) {
			var marker = new GMarker(point, {icon: maxxerIcon, title: name});
			marker.user = name;
			marker.id = userid
			marker.type = "marker";

			// we got an id through the URL, act as if you clicked
			// the marker
			if(find_id == userid) {
				click_id = i;
				window.setTimeout(function() {
					myclick(click_id);
				}, 2000);
			}

			// save the info we need to use later for the side_bar
			gmarkers[i] = marker;

			// add a line to the side_bar html
			sidebar_html += '<a  href="javascript:myclick(' + i + ')">' + name + '</a> &nbsp;';
			i++;
			return marker;
		}

		// create a cluster point
		function createCluster(point, num_users, swx, swy, nex, ney) {
			var cluster = new GMarker(point, {icon: clusterIcon, title: num_users + " maxxers"});
			cluster.type = "cluster";

			// create some extra space around the markers
			var x_center = nex + ((nex-swx)/2);
			var y_center = ney + ((ney-swy)/2);	
			cluster.swx = swx - (x_center - swx);
			cluster.swy = swy - (y_center - swy);
			cluster.nex = nex + (x_center - swx);
			cluster.ney = ney + (y_center - swy);
			return(cluster);
		}

		// start the map
		map = new GMap2(document.getElementById("map"));
		map.addMapType(G_PHYSICAL_MAP);
		map.addControl(new GLargeMapControl());
		map.addControl(new GMapTypeControl());
		map.addControl(new GScaleControl());
		map.enableScrollWheelZoom();

		var pos = new GControlPosition(G_ANCHOR_BOTTOM_LEFT, new GSize(200,5));
		pos.apply(document.getElementById("map_edit"));
		map.getContainer().appendChild(document.getElementById("map_edit"));
		map.setCenter(new GLatLng(0,0), 1, G_SATELLITE_MAP);

		var mgrOptions = { borderPadding: 5};
		mgr = new MarkerManager(map, mgrOptions);

		// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
		GDownloadUrl("/offensive/map/get_coordinates.php", function(data) {
			var xml = GXml.parse(data);
			var markers = xml.documentElement.getElementsByTagName("marker");
			var mgr_markers = [];
			var mgr_clusters = [];
			var icon;
			
			for (var k=0; k < 18; k++) {
				mgr_markers[k] = [];
				mgr_clusters[k] = [];
			}

			for (var j = 0; j < markers.length; j++) {
				var name = markers[j].getAttribute("user");
				var userid = markers[j].getAttribute("userid");
				var minzoom = parseInt(markers[j].getAttribute("minzoom"));

				var point = new GLatLng(parseFloat(markers[j].getAttribute("lat")), parseFloat(markers[j].getAttribute("lon")));
				var marker = createMarker(point, name, userid);
				mgr_markers[minzoom].push(marker);
			}
			// put the assembled side_bar_html contents into the side_bar div
			document.getElementById("sidebar").innerHTML = sidebar_html;

			// grab all the cluster icons
			var clusters = xml.documentElement.getElementsByTagName("cluster");
			for (var j = 0; j < clusters.length; j++) {
				var level = parseInt(clusters[j].getAttribute("level"));
				var point = new GLatLng(parseFloat(clusters[j].getAttribute("lat")), parseFloat(clusters[j].getAttribute("lon")));
				var cluster = createCluster(point,
							    clusters[j].getAttribute("num_users"),
							    parseFloat(clusters[j].getAttribute("swx")),
							    parseFloat(clusters[j].getAttribute("swy")),
							    parseFloat(clusters[j].getAttribute("nex")),
							    parseFloat(clusters[j].getAttribute("ney"))
							   );
				mgr_clusters[level].push(cluster);
			}

			// draw the markers
			for(i=0; i< 18; i++ ) {
				mgr.addMarkers(mgr_markers[i],i);
				mgr.addMarkers(mgr_clusters[i],i,i);
			}
			mgr.refresh();
		});

		// add a listener to the map that checks if a marker was clicked
		// if so, grab the info for this marker's tmbo user 
		GEvent.addListener(map, "click", function(overlay, point, latlng) {
			var id = "";
			if (overlay){ // marker clicked
				if (overlay instanceof GMarker) {
					if(overlay.type == "cluster") {
						var bounds = new GLatLngBounds();
						var sw_point = new GLatLng(overlay.swx,overlay.swy);
						var ne_point = new GLatLng(overlay.nex,overlay.ney);
						bounds.extend(sw_point);
						bounds.extend(ne_point);

						map.setZoom(map.getBoundsZoomLevel(bounds));
						map.setCenter(bounds.getCenter());
					} else {
						// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
						GDownloadUrl("/offensive/map/get_userinfo.php?user=" + overlay.id, function(data) {
							overlay.openInfoWindowHtml(data);   // open InfoWindow
						
						});
					}
				}
			} else if (point) {	// background (dont do anything for now)
			}
		});
	} else {
		alert("Your browser is not compatible");
	}
}
		
function editmap(user) {
	if (GBrowserIsCompatible()) {
		map = new GMap2(document.getElementById("map"));
		map.addControl(new GLargeMapControl());
		map.addControl(new GMapTypeControl());
		map.enableScrollWheelZoom();
		map.setCenter(new GLatLng(0,0), 1, G_SATELLITE_MAP);
		
		var pos = new GControlPosition(G_ANCHOR_BOTTOM_LEFT, new GSize(200,5));
		pos.apply(document.getElementById("map_edit"));
		map.getContainer().appendChild(document.getElementById("map_edit"));
		// add a GeoCoder object for address searches
		geocoder = new GClientGeocoder();

		// create a marker (if a user has one). No need to do a sidebar
		function createMarker(point, name, userid, myIcon) {
			var marker = new GMarker(point, {icon: maxxerIcon, title: name});
			marker.user = name;
			marker.id = userid;
			return marker;
		}

		// see if this user has a marker already. 
		// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
		GDownloadUrl("/offensive/map/get_coordinate.php?user=" + user, function(data) {
			var xml = GXml.parse(data);
			var markers = xml.documentElement.getElementsByTagName("marker");
			for (var i = 0; i < markers.length; i++) {
				var name = markers[i].getAttribute("user");
				var userid = markers[i].getAttribute("userid");
				var icon = maxxerIcon;
				var point = new GLatLng(parseFloat(markers[i].getAttribute("lat")), parseFloat(markers[i].getAttribute("lon")));
				map.setCenter(point,8,G_SATELLITE_MAP);
				var marker = createMarker(point, name, userid, icon);
				map.addOverlay(marker);
			}
		});

		// this listener had two functions.
		// 1. check if a point was clicked, if so, delete it.
		// 2. if a map was clicked, remove all points and add a new one
		
		GEvent.addListener(map, "click", function(overlay, point) {
			var id = "";
			if (overlay){ // marker clicked
				if (overlay instanceof GMarker) {
					map.removeOverlay(overlay);
					// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
					GDownloadUrl("/offensive/map/user_delete.php", function(data) {
					});
				}
			} else if (point) { // background clicked
				// first remove all overlays.
				map.clearOverlays();

				var m = new GMarker(point, {icon: maxxerIcon});
				map.addOverlay(m);
				var lat = m.getPoint().lat();
				var lng = m.getPoint().lng();
				// XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
				GDownloadUrl("/offensive/map/user_add.php?lat=" + lat + "&long=" + lng, function(data) {
				});
			}
		});
	} else {
		alert("Your browser is not compatible");
	}
}

function showAddress(address) {
	if (geocoder) {
		geocoder.getLatLng(address, function(point) {
			if (!point) {
				alert(address + " not found");
			} else {
				map.setCenter(point, 15);
			}
		});
	}
}

window.onunload = function() {
	GUnload();
};
