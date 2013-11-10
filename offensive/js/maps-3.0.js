/*
 +-----------------------------------------------------------------------+
 | Google Maps                                                           |
 |                                                                       |
 | Licensed under the GNU General Public License version 3               |
 |                                                                       |
 +-----------------------------------------------------------------------+
 | Author: Cor Bosman (cor@in.ter.net)                                   |
 +-----------------------------------------------------------------------+
*/

var maxxerMap;

function map() {
 
    var me;                                                     // to reach this object from event handlers
    var map;                                                    // map object
    var mgr;                                                    // markermanager object 
    var mgr_markers  = [];                                      // marker manager array
    var mgr_clusters = [];                                      // cluster manager array
    var maxxerIcon   = '/offensive/graphics/map_man_blue.png';  // normal icon
    var clusterIcon  = "/offensive/graphics/map_cluster.png";   // cluster icon
    var sidebarHtml  = "";                                      // sidebar html
    var gmarkers     = [];                                      // marker array 
    var infoWindow   = false;                                   // infowindow object
    var markerIndex  = 0;
 
    /**
     * initialise the map
     */
    this.init = function () {
        // remember this object, else we cant reach it from closures.
        me = this;

        // fire up a map object
        map = new google.maps.Map(
            document.getElementById('map'), {
            center: new google.maps.LatLng(37, -45),
            zoom: 2,
            mapTypeId: google.maps.MapTypeId.HYBRID
        });

        // insert a new control into the map
        map.controls[google.maps.ControlPosition.BOTTOM_LEFT].push(document.getElementById("map_edit"));

        // wait for the map to be ready, and then load all markers
        google.maps.event.addListenerOnce(map, 'projection_changed', function() {
            me.loadMarkers();
        });
    }

    /**
     * map is ready, load all the markers
     */
    this.loadMarkers = function() { 

        // load the markers from the database
        $.get("/offensive/map/get_coordinates.php", function(data) {
            var markers         = data.markers;
            
            // create arrays for the markermanager
            for (var k=0; k < 18; k++) {
                mgr_markers[k]  = [];
                mgr_clusters[k] = [];
            }

            // loop over all markers 
            for (var j = 0; j < markers.length; j++) {
                var name     = markers[j].user;
                var userid   = markers[j].userid;
                var minzoom  = parseInt(markers[j].minzoom);
                var position = new google.maps.LatLng(parseFloat(markers[j].lat), parseFloat(markers[j].lon));

                // create the marker
                var marker = me.createMarker(position, name, userid);

                // add the marker to the markermanager
                mgr_markers[minzoom].push(marker);
            }
            // put the assembled side_bar_html contents into the side_bar div
            document.getElementById("sidebar").innerHTML = sidebarHtml;

            // grab all the cluster icons
            var clusters = data.clusters;
            for (var j = 0; j < clusters.length; j++) {
                var level = parseInt(clusters[j].level);
                var position = new google.maps.LatLng(parseFloat(clusters[j].lat), parseFloat(clusters[j].lon));
                var cluster = me.createCluster(position,
                                clusters[j].num_users,
                                parseFloat(clusters[j].swx),
                                parseFloat(clusters[j].swy),
                                parseFloat(clusters[j].nex),
                                parseFloat(clusters[j].ney)
                            );
                mgr_clusters[level].push(cluster);
            }

            // create a markermanager object
            mgr = new MarkerManager(map, {
                borderPadding: 5
            });

            // wait for the markermanager to initialise before continuing.
            google.maps.event.addListener(mgr, 'loaded', function() {
                // add the markers to the markermanager
                for(j=0; j< 18; j++ ) {
                    // add markers on this level, visible to highest zoomlevel
                    if(mgr_markers[j].length > 0)  mgr.addMarkers(mgr_markers[j],j,19);

                    // add clusters on this level, each clustericon is only visible on 1 level
                    if(mgr_clusters[j].length > 0) mgr.addMarkers(mgr_clusters[j],j,j);
                }

                // refresh the markermanager to show the markers
                mgr.refresh();    
            }); 

        });
    }

    /**
     * create a marker
     * @param  {object} position map coordinates
     * @param  {string} name     username
     * @param  {int}    userid   userid
     * @return {object}          Marker object
     */
    this.createMarker = function(position, name, userid) {      
        // create the marker
        var marker = new google.maps.Marker({
            icon:       maxxerIcon,
            position:   position,
            title:      name
        });

        // add some extra data
        marker.user = name;
        marker.id   = userid
        marker.type = "marker";

        // listen to marker click event to show the infowindow
        google.maps.event.addListener(marker, "click", function(e) {
            me.showInfo(this);
        });

        // keep an array of markers for click events elsewhere in the ui.
        gmarkers[markerIndex] = marker;

        // add a line to the side_bar html
        sidebarHtml += '<a  href="javascript:maxxerMap.myclick(' + markerIndex + ')">' + name + '</a> &nbsp;';

        markerIndex++;
        return marker;
    }

    /**
     * create a cluster
     * @param {object} position  cluster coordinates
     * @param {int}    num_users number of maxxers in this cluster
     * @param {float}  swx       cluster area coordinate
     * @param {float}  swy       cluster area coordinate
     * @param {float}  nex       cluster area coordinate
     * @param {float}  ney       cluster area coordinate
     * @return {object}          Marker object
     */
    this.createCluster = function(position, num_users, swx, swy, nex, ney) {
        // create a title for mouse hover
        var title = num_users + ' maxxers';

        // new cluster 
        var cluster = new google.maps.Marker({
            icon:       clusterIcon,
            position:   position,
            title:      title
        });

        // set type
        cluster.type = "cluster";

        // create some extra space around the markers
        var x_center = nex + ((nex-swx)/2);
        var y_center = ney + ((ney-swy)/2); 
        cluster.swx = swx - (x_center - swx);
        cluster.swy = swy - (y_center - swy);
        cluster.nex = nex + (x_center - swx);
        cluster.ney = ney + (y_center - swy);

        // on click, zoom into this cluster
        google.maps.event.addListener(cluster, "click", function(e) {
            var sw_point = new google.maps.LatLng(this.swx, this.swy);
            var ne_point = new google.maps.LatLng(this.nex, this.ney);
            var bounds = new google.maps.LatLngBounds(
                sw_point,
                ne_point
            );

            // extend the area a bit for border markers
            bounds.extend(sw_point);
            bounds.extend(ne_point);
            
            // get the zoomlevel that we need to show all points on the map
            var zoomLevel = me.getZoomByBounds(bounds);

            // move to this zoomlevel
            map.setZoom(zoomLevel);
            map.setCenter(bounds.getCenter());
        });

        return(cluster);
    }

    /**
     * find the zoomlevel we need to show all points in a cluster
     * @param  {object} bounds  boundaries of this cluster
     * @return {int}            zoomlevel
     */
    this.getZoomByBounds = function(bounds) {
        var maxZoom = map.mapTypes.get( map.getMapTypeId() ).maxZoom || 19 ;
        var minZoom = map.mapTypes.get( map.getMapTypeId() ).minZoom || 0 ;

        var ne= map.getProjection().fromLatLngToPoint( bounds.getNorthEast() );
        var sw= map.getProjection().fromLatLngToPoint( bounds.getSouthWest() ); 

        var worldCoordWidth = Math.abs(ne.x-sw.x);
        var worldCoordHeight = Math.abs(ne.y-sw.y);

        //Fit padding in pixels 
        var fitPad = 40;

        for( var zoom = maxZoom; zoom >= minZoom; --zoom ){ 
            if( worldCoordWidth*(1<<zoom)+2*fitPad < $(map.getDiv()).width() && 
                worldCoordHeight*(1<<zoom)+2*fitPad < $(map.getDiv()).height() )
                return zoom;
        }
        return 0;
    }

    /**
     * display the infowindow for this marker
     * @param  {object} marker marker object
     */
    this.showInfo = function(marker) {
        // get the user info from the server
        $.get("/offensive/map/get_userinfo.php?user=" + marker.id, function(data) {
            if(!me.infoWindow) {
                me.infoWindow = new google.maps.InfoWindow({ maxWidth: 300});
            }
            //me.infoWindow.close();
            
            me.infoWindow.setOptions({
                content: data,
                maxWidth: 300
            });

            me.infoWindow.open(map, marker);
        });
    }

    /**
     * for clicking on a posse link
     * we need to go through all markers to find a marker with this userid
     * @param  {int} i    index of this posse member
     */
    this.posseClick = function(i) {
        for(var j=0; j<gmarkers.length; j++) {
            if(gmarkers[j].id == i) {
                me.myclick(j);
                break;
            }
        }
    }

    /**
     * geocode an address and zoom the map to that position
     * @param  {string} address
     */
    this.showAddress = function(address) {
        // add a GeoCoder object for address searches
        geocoder = new google.maps.Geocoder();

        geocoder.geocode( { 'address': address}, function(results, status) {
            if (status == google.maps.GeocoderStatus.OK) {
                map.setCenter(results[0].geometry.location);
                map.setZoom(18);
            } else {
                alert('Geocode was not successful for the following reason: ' + status);
            }
        });
    }

    /**
     * used when you click on a link in the area under the map
     * @param  {int} i   index of marker in the gmarkers array
     */
    this.myclick = function(i)
    {
        // the marker we clicked
        var marker = gmarkers[i];

        // center and zoom on the marker
        map.setCenter(marker.getPosition());
        map.setZoom(8);

        // show the info window
        this.showInfo(marker);
    }



    this.edit = function(user) {
        // remember this object, else we cant reach it from closures.
        me = this;

        // create a new map
        map = new google.maps.Map(
            document.getElementById('map'), {
            center: new google.maps.LatLng(37.4419, -122.1419),
            zoom: 2,
            center: new google.maps.LatLng(0,0),
            mapTypeId: google.maps.MapTypeId.HYBRID
        });

        // insert the edit map button into the map
        map.controls[google.maps.ControlPosition.BOTTOM_LEFT].push(document.getElementById("map_edit"));

        // see if this user has a marker already. 
        $.get("/offensive/map/get_coordinate.php", function(data) {
            var markers = data.markers;

            for (var i = 0; i < markers.length; i++) {
                position = new google.maps.LatLng(parseFloat(markers[i].lat), parseFloat(markers[i].lon));
                me.newMarker(position);
            }
        });

        /**
         * listener for map clicks
         * 1. remove all existing markers (should only be 1)
         * 2. create new marker on the clicked spot
         * 3. save marker 
         */
        google.maps.event.addListener(map, "click", function(e) {
            // delete all markers
            me.deleteMarkers();

            // create a marker
            marker = me.newMarker(e.latLng);

            // and save the new position in the DB
            $.get("/offensive/map/user_add.php?lat=" + e.latLng.lat() + "&long=" + e.latLng.lng(), function(data) {});

        });
    }

    /**
     * delete all markers
     */
    this.deleteMarkers = function() {
        for (var i = 0; i < gmarkers.length; i++) {
          gmarkers[i].setMap(null);
        }
        gmarkers = [];
    }

    /**
     * create a new marker
     * @param  {latLng} position
     * @return {Marker}        
     */
    this.newMarker = function(position) {
        // create a new marker
        var marker = new google.maps.Marker({
            icon:       maxxerIcon,
            position:   position,
            map:        map
        });

        // add a listener for this marker to delete it when clicked
        google.maps.event.addListener(marker, "click", function() {
            // delete all markers
            me.deleteMarkers();

            // XXX: eventually this should use the API, once the SSL issues with the lib are sorted.
            $.get("/offensive/map/user_delete.php", function(data) {});
        });

        // save this marker 
        gmarkers.push(marker);

        return marker;
    }

};

/**
 * wait for the page to load completely before starting up the map
 */
$(document).ready(function () {
    maxxerMap = new map();
    mapEdit ? maxxerMap.edit() : maxxerMap.init();
});

