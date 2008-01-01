<?
	set_include_path("../..");
	require_once( 'offensive/assets/header.inc' );

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../" );
	}

	require_once( 'admin/mysqlConnectionInfo.inc' );

	$x = array_key_exists("map_x", $_REQUEST) ? $_REQUEST['map_x'] : "";
	$y = array_key_exists("map_y", $_REQUEST) ? $_REQUEST['map_y'] : "";

	if( is_numeric( $x ) && is_numeric( $y ) ) {
		setMaxxerLoc( $_SESSION['userid'], $x, $y );
		touch( "last_update.txt" );
		header( "Location: ./?p=1" );
		exit;
	}

	function setMaxxerLoc( $maxxerid, $x, $y ) {
		global $link;

		if( ! is_numeric( $maxxerid ) ) {
			return;
		}

		if(!isset($link) || !$link) $link = openDbConnection();
		$sql = "replace into maxxer_locations (userid, x, y) values ( $maxxerid, $x, $y )";
		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		
		if( file_exists( "users/$maxxerid.jpg" ) ) {
			unlink( "users/$maxxerid.jpg" );
		}
	}
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 7.0.3">
	
	
</head>


<body bgcolor="#ffffff">

	<div style="margin:18px;">Click the map to set your location, or <a href="./">click here</a> to cancel.</div>

	<form action="<?= $_SERVER['PHP_SELF'] ?>">
		<input type="image" name="map" src="tmbomap.jpg"/>
	</form>

</body>
</html>
