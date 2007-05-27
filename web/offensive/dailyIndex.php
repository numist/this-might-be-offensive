<?
	session_start();
	require_once( 'public.php' );
	
	
	$index = isset( $_REQUEST['id'] ) ? $_REQUEST['id'] : 0;
	
	$filename = $images[ $index ];

	if( isset( $_REQUEST['list'] ) ) {
		$_SESSION['dailylist'] = $_REQUEST['list'];
	}

	$showList = isset( $_SESSION['dailylist'] ) ? $_SESSION['dailylist'] : 1;

	function toggleListLink() {
		global $showList, $index;
		if( $showList == 1 ) {
			?><a class="navlink" href="<?= $_SERVER['PHP_SELF']?>?id=<?= $index ?>&list=0">hide list</a><?
		}
		else {
			?><a class="navlink" href="<?= $_SERVER['PHP_SELF']?>?id=<?= $index ?>&list=1">show list</a><?
		}
	}

	function thumbnailFor( $filename ) {
		$th = "./thumbs/th-$filename";
		if( file_exists( $th ) ) {
			return "./thumbs/th-" . rawurlencode( $filename ) ;
		}
		return "assets/previewNotAvailable.gif";
	}

	function emitList() {
		global $showList, $images, $index;
		if( $showList == 1) {
		?>
			<div id="list">
				<?
					for( $i = 0; $i < sizeof( $images ); $i++ ) {
						
						$thumbnail = thumbnailFor( $images[ $i ] );
					
						$mouseOver = thumbnail ? "showThumbnail( '$thumbnail', this )" : "";
					
						?> <div class="file" onmouseover="<?= $mouseOver ?>" onmouseout="hideThumbnail()"> <?= $i + 1 ?>.<?
						if( $i <> $index ) {
							?> <a href="<?= $_SERVER['PHP_SELF'] ?>?id=<?= $i ?>"><?= htmlentities( $images[$i] ) ?></a><br/> <?
						}
						else {
							?> &raquo; <b><?= htmlentities( $images[$i] ) ?></b><br/> <?
						}
						?> </div> <?
					}
				?>
			</div>
		<?
		}
	}

?>


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">

<html>
	<head>
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>[ this might be offensive ] : <?= $filename ?></title>
		<link rel="stylesheet" type="text/css" href="assets/oldskool.css"/>
		<link rel="stylesheet" type="text/css" href="assets/styles.php"/>
		<style type="text/css">
			body {
				background:white;
				background-image: url( assets/dailydigest.png );
				background-position: top right;
				background-repeat: no-repeat;
			}
			
			.navlink {
				margin-right: 14px;
			}
			
			.info {
				color:#999999;
			}
			
			#list {
				float:left;
				text-align:left;
				width:250px;
				font-size:10px;
				white-space:nowrap;
				overflow:hidden;
				margin-right:12px;
				margin-bottom:100px;
			}
			
			#content {
				width:auto;
				white-space:nowrap;
			}
			
			#thumb {
				position:absolute;
				left:250px; /* width of the list container. */
				padding:0px;
				border:2px solid white;
				background:white;
				display:none;
			}

			.file {
				padding:3px;
				background:#eeeeee;
				border-bottom:1px solid white;
			}
			
			.file.hover {
				background:#ccccff;
			}
		</style>
		<script type="text/javascript">
			self.file_id = "";
			
			// prevent sites from hosting this page in a frame;
			if( window != top ) {
				top.location.href = window.location.href;
			}
			
			
			function showThumbnail( imgSrc, srcElement ) {
				var img = document.getElementById( 'thumbnail' );
				if( img ) {
					img.src = imgSrc;
				}
				var thumbContainer = document.getElementById( 'thumb' );
				thumbContainer.style.display = "block";
				thumbContainer.style.top = srcElement.offsetTop + "px";
//				thumbContainer.style.left = (srcElement.offsetLeft + srcElement.offsetWidth) + "px";				
				
			}
			
			function hideThumbnail() {
				var img = document.getElementById( 'thumbnail' );
				if( img ) {
					img.src = 'assets/loadingThumb.png';
				}
				document.getElementById( 'thumb' ).style.display="none";
			}

		</script>
		<script type="text/javascript" src="assets/offensive.js"></script>

	</head>
	<body onload="doOnloadStuff()" onkeydown="return handleKeyDown( event );">
	
		<div id="thumb">
			<img src="./graphics/loadingThumb.png" id="thumbnail"/>
		</div>
	
		<div id="content">
			<p>
				<? $style = ( $index > 0 ) ? "" : "visibility:hidden" ?>
				<a class="navlink" style="<?= $style ?>" id="previous" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $index - 1 ?>">previous</a>
				<?
					if( $index < sizeof( $images ) - 1 ) { ?>
						<a class="navlink" id="next" href="<? echo $_SERVER['PHP_SELF']?>?id=<?= $index + 1 ?>">next</a>
				<? }
				toggleListLink() ?>
			</p>

			<? emitList() ?>
			<div class="image">
				<p><span class="info">(<?= $index + 1 . "/" . sizeof( $images ) ?>)</span> <?= $filename ?></p>
				<a href="images/<? echo rawurlencode( $filename )?>" target="_blank"><img src="images/<? echo rawurlencode( $filename )?>" style="border:none"/></a>
			</div>
			<br/><br/>
		</div>
	</body>
</html>
