<?
	set_include_path("../..");
	require_once( 'offensive/assets/header.inc' );

	$usrid = $_SESSION['userid'];
	if( ! is_numeric( $usrid ) ) {
		session_unset();
		header( "Location: ../" );
	}

	require_once( 'admin/mysqlConnectionInfo.inc' );
	
	$cells = array();
	$cellSize = 100;
	
	function emitCellDetails() {
		global $cells, $cellSize, $link;
		$sql = "SELECT maxxer_locations.userid,
						floor(x/$cellSize) as xCell,
						floor(y/$cellSize) as yCell,
						username
		FROM maxxer_locations, users
		WHERE users.userid = maxxer_locations.userid
		order by xCell, yCell, username";
	
		if(!isset($link) || !$link) $link = openDbConnection();
	
		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		
		$xCell = -1;
		$yCell = -1;
		$itemOpen = false;
		while( $row = mysql_fetch_assoc( $result ) ) {
			if( ($row[ 'xCell' ] != $xCell) || ($row[ 'yCell' ] != $yCell) ) {
				$xCell = $row[ 'xCell' ];
				$yCell = $row[ 'yCell' ];			
				if( $itemOpen ) {
					closeCell();
				}
				openCell( $xCell, $yCell );
				array_push( $cells, array($xCell, $yCell));
				$itemOpen = true;
			}
			emitCellItem( $row['username'] );
		}
	
		if( $itemOpen ) {
			closeCell();
		}
	}

	function openCell( $x, $y ) {
		?>
<div class="cellDetail" id="<?= idFor($x, $y ) ?>">
		<?
	}
	
	function closeCell() {
		?>
</div>
		<?
	}
	
	function emitCellItem( $name ) {
		?>
	<?= $name ?><br/>
		<?
	}
	
	function idFor( $x, $y ) {
		return "$x.$y";
	}
	
	function emitImageMap() {
		global $cells, $cellSize;
	?>
<map name="details">
	<?
		foreach( $cells as $c ) {
			$x = $c[0];
			$y = $c[1];			
			?><area shape="RECT"
					COORDS="<?= $x * $cellSize ?>,<?= $y * $cellSize ?> <?= ($x + 1) * $cellSize ?>,<?= ($y + 1) * $cellSize ?>"
					HREF="#"
					onclick="return false"
					onmouseover="showDetail( event, <?= idFor($x,$y) ?> );"
					onmouseout="hideDetails();"/>
			<?
		}
	?>
	
</map>
<?
	}	
	
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
	<title>Untitled</title>
	<meta name="generator" content="BBEdit 8.2">
	<style type="text/css">
		.cellDetail {
			border:1px solid black;
			margin:4px;
			padding:4px;
			width:150px;
		}
		
		#details {
			position:absolute;
			background:white;
		}
		
		body {
			font-family:verdana;
			font-size:11px;
		}
	</style>
	<script type="text/javascript">
		
		function showDetail( event, id ) {
			var detailCell = document.getElementById( id );
			if( detailCell ) {
				var container = document.getElementById( "details" );
				container.appendChild( detailCell.cloneNode( true ) );
				positionElement( container );
			}
		}
		
		function hideDetails() {
			var container = document.getElementById( "details" );
			while( container.firstChild ) {
				container.removeChild( container.firstChild );
			}
		}
		
	function positionElement( noteElem ) {
	
		var windowDim = windowInfo();
		var top = -(windowDim.yScroll);
		var left = windowDim.xScroll;
		
		var right = left + elementWidth( noteElem );
		var bottom = top + elementHeight( noteElem );
	
		if( right > windowDim.width ) {
			var adjustment = (right - windowDim.width) + 18;
			left -= adjustment;
		}
		
		if( bottom > windowDim.height ) {
			var adjustment = (bottom - windowDim.height) + 18;
			top -= adjustment;
		}
		
		noteElem.style.top = top + "px";
		noteElem.style.left =  left + "px";
	}

	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// elementWidth-- returns the pixel width of the given element
	////////////////////////////////////////////////////////////////////////////////////////////////////
	function elementWidth( elem ) {
		return elem.offsetWidth;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// elementHeight-- returns the pixel height of the given element
	////////////////////////////////////////////////////////////////////////////////////////////////////
	function elementHeight( elem ) {
		return elem.offsetHeight;
	}

	

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// windowInfo-- returns a struct containing size and scroll information about the window.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	function windowInfo() {	
		var width = 0;
		var height = 0;
		var yScroll = 0;
		var xScroll = 0;
		
		// get window dimensions
		if( typeof( window.innerWidth ) == 'number' ) {
			// moz
			width = window.innerWidth;
			height = window.innerHeight;
		}
		else if( document.documentElement && ( document.documentElement.clientWidth || document.documentElement.clientHeight ) ) {
			//IE 6+ in 'standards compliant mode'
			width = document.documentElement.clientWidth;
			height = document.documentElement.clientHeight;
		}
		else if( document.body && ( document.body.clientWidth || document.body.clientHeight ) ) {
			//IE 4 compatible
			width = document.body.clientWidth;
			height = document.body.clientHeight;
		}
		
		// get scroll position
		yScroll = getScrollY();
		xScroll = getScrollX();
		
		return { height:height, width:width, yScroll:yScroll, xScroll:xScroll };
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// getScrollX-- returns the current horizontal scrolling in pixels. note that we're using
	// a fixed body to accomodate floating column headings, so this function references (and expects to
	// find) a "bodycontent" element.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	function getScrollX() {
		var bodycontent = document.body;
		if( ! bodycontent ) {
			return 0;
		}
		return window.scrollX ? bodycontent.scrollX :  bodycontent.scrollLeft;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// getScrollY-- returns the current vertical scrolling in pixels. note that we're using
	////////////////////////////////////////////////////////////////////////////////////////////////////
	function getScrollY() {
		var bodycontent = document.body;
		if( ! bodycontent ) {
			return 0;
		}
		return window.scrollY ? bodycontent.scrollY :  bodycontent.scrollTop;
	}
	

		
	</script>
</head>
<body>

<div style="display:none">
	<? emitCellDetails(); ?>
</div>

<div id="details">
	
</div>

<img src="comp.jpg" width="5000" height="3000" usemap="details"/>
	
<?	emitImageMap(); ?>
	
</body>
</html>
