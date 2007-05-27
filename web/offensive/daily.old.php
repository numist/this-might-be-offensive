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

	function emitList() {
		global $showList, $images, $index;
		if( $showList == 1) {
		?>
			<div id="list">
				<?
					for( $i = 0; $i < sizeof( $images ); $i++ ) {
						?> <div class="file"> <?= $i + 1 ?>.<?
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
		<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>
		<link rel="stylesheet" type="text/css" href="pages/styles.php"/>
		<style type="text/css">
			body {
				background:white;
				background-image: url( graphics/dailydigest.png );
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
			}
			
			#content {
				width:auto;
				white-space:nowrap;
			}
			
			.file {
				padding:3px;
				background:#eeeeee;
				border-bottom:1px solid white;
			}
		</style>
		<script type="text/javascript">
			self.file_id = "";
			
			// prevent sites from hosting this page in a frame;
			if( window != top ) {
				top.location.href = window.location.href;
			}
		</script>
		<script type="text/javascript" src="pages/offensive.js"></script>

	</head>
	<body onload="doOnloadStuff()" onkeydown="return handleKeyDown( event );">
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
				<a href="images/picpile/<? echo rawurlencode( $filename )?>" target="_blank"><img src="http://images.themaxx.com/mirror.php/offensive/images/picpile/<? echo rawurlencode( $filename )?>" style="border:none"/></a>
			</div>
			<br/><br/>
		</div>
	</body>
</html>
