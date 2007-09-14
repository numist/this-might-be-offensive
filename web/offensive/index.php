<?

	ob_start();
	session_start();

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ./logn.php?redirect=" . urlencode( $_SERVER['PHP_SELF'] . "?" . $_SERVER['QUERY_STRING'] ));
		exit;
	}

#	if( $_SESSION['userid'] == 2250 ) {
#		header( "Location: ./logout.php" );
#		exit;
#	}

	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	if( isset( $_REQUEST['c'] ) ) {
		$c = $_REQUEST['c'];
	}
	else {
		$c = ($_COOKIE["thumbnails"] == "yes") ? "thumbs" : "images";
	}

	if( ! file_exists( "content/{$c}.php" ) ) {
		$c = "images";
	}

	include( "content/{$c}.php" );

	if( function_exists( 'start' ) ) {
		start();
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">


<html>
<head>
	<meta http-equiv="Content-Type" content="text/html;charset=ISO-8859-1">
	<title>
	<?
		if( function_exists( 'title' ) ) {
			echo title();
		}
		else {
			echo "[ this might be offensive ]";
		}
	?>
	</title>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<meta name="generator" content="BBEdit 6.0.2">
	<link rel="icon" href="/favicon.ico" />
	<link rel="shortcut icon" href="/favicon.ico" />
	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>
	<link rel="stylesheet" type="text/css" href="nsfw.css.php"/>
	
<style type="text/css">

	/* override definitions in oldskool.css to make main content area wider */	

	#content {
		margin-left:auto;
		margin-right:auto;
		text-align:left;
		line-height: 15px;
		width:771px;
	}
	
	#rightcol {
		width:584px;
		float:left;
		margin-right: auto;
		margin-left: 0px;
	}
	
</style>

<?
	if( function_exists( 'head' ) ) {
		head();
	}
?>

</head>

<body bgcolor="#333366" link="#000066" vlink="#000033">

 <?php include( $DOCUMENT_ROOT . "/includes/headerbuttons.txt" );?>
<br>

	<div id="content">
	
		<div id="titleimg"><a href="./"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></a></div>

	
		<div id="leftcol">

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">log in</div>
					<div class="bluebox">
						<p>you are logged in as <b><a href="index.php?c=user&userid=<?php echo $_SESSION['userid'] ?>"><?php echo $_SESSION['username'] ?></a>.</b></p>
						
						<p><a href="index.php?c=upload">upload</a></p>
						
						<p><a href="./?c=subscriptions">subscribed threads</a></p>

						<p><a href="logout.php">log out</a></p>
					</div>
				<div class="blackbar"></div>
			</div>

			<? 
				if( function_exists( 'sidebar' ) ) {
					sidebar();
				}
				
				
				unread();

			?>

<!--
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">help.</div>
					<div class="bluebox">
						<div style="text-align:center">						
							<a href="http://www.redcross.org"><img src="graphics/redcross.gif" alt="the american red cross" title="the american red cross" style="border:none"/></a>							
						</div>
					</div>
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">shirts still available:</div>
					<div class="bluebox">
						<a href="http://tmbo.org/offensive/index.php?c=comments&fileid=113996"><img src="graphics/tmboshirt.png" width="150" height="164" alt="shirt" style="border:none"/></a>
					</div>
				<div class="blackbar"></div>
			</div>
-->

<? /*
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">nice shot!</div>
					<div class="bluebox" style="text-align:center">
						<a href="?c=shots"><img src="graphics/shot.png" width="113" height="158" alt="buy me a shot" style="border:none"/></a>
					</div>
					<div class="heading" style="text-align:center">
						<a class="orange" href="?c=shots">[+] shot glasses: $4.99</a>
					</div>

				<div class="blackbar"></div>
			</div>
*/ ?>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">get info:</div>
					<div class="bluebox">
						<p><a href="./map/gmap.php">maxxer world map</a></p>
						<p><a href="<? echo $_SERVER['PHP_SELF'] ?>?c=referral">invite a friend</a></p>						
						<p><a href="<? echo $_SERVER['PHP_SELF'] ?>?c=faq">FAQ, rules, etc.</a></p>
						<p><a href="http://tmboradio.com">tmboradio.com</a></p>
						<!-- <p><a href="./?c=stats">stats</a></p> -->						
					</div>
				<div class="blackbar"></div>
			</div>


			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">bandwidth isn't free:</div>
					<div class="bluebox">
						<div style="text-align:center">						
							<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
								<input type="hidden" name="cmd" value="_xclick">
								<input type="hidden" name="business" value="bandwidth@thismight.be">
								<input type="hidden" name="item_name" value="[ this might be offensive ] bandwidth">
								<input type="hidden" name="no_shipping" value="1">
								<input type="hidden" name="shipping" value="0">								
								<input type="hidden" name="no_note" value="1">
								<input type="hidden" name="currency_code" value="USD">
								<input type="hidden" name="tax" value="0">
								<input type="image" src="graphics/paypal.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
							</form>
						</div>
						<br/>
						<div style="text-align:center">						
							<a href="./?c=ppsub"><img src="graphics/paypal_subscribe.gif" border="0"/></a>
						</div>
					</div>
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">web hosting provided by:</div>
					<div class="bluebox" style="text-align:center">
						<a href="http://tengun.net">tengun.net</a>
					</div>
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="heading">archives:</div>
				<div class="bluebox">
					<?php include( 'ziplist.txt' ); ?>
				</div>

				<div class="heading" style="text-align:center">
					<span class='orange'>
						<a class="orange" href="./?c=hof">hall of fame</a>
					</span>
				</div>
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="heading">rss:</div>
				<div class="bluebox" style="text-align:center">
					<a href="pic_rss.php"><img src="graphics/rss_pics.gif" border="0" alt="rss: pics" width="77" height="15" style="margin-bottom:6px"></a><br/>
					<a href="zip_rss.php"><img src="graphics/rss_zips.gif" border="0" alt="rss: zips" width="77" height="15"></a>
				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">contact:</div>
					<div class="bluebox">
						<a href="/contact/">email</a><br>
						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
				<div class="blackbar"></div>
			</div>
			
		</div> <!-- end left column -->
		
		<div id="rightcol">
			<div class="contentbox">
				<div class="blackbar"></div>

					<?
						if( function_exists( 'body' ) ) {
							body();
						}
					?>

				<div class="blackbar"></div>
			</div>
		</div>

	</div>

<br clear="all">
<div class="textlinks" style="text-align:center">
	<hr width="300" />

	<? include '../includes/footer.txt' ?>

	<div class="textlinks">contents copyright &copy; 1997-<?= date("Y") ?> <a href="/contact/" class="textlinks" onmouseover='window.status="[ connect ]"; return true' onmouseout='window.status=""'>Ray Hatfield</a>. All rights reserved.</div>
</div>
<br />

</body>
</html>
<?
	ob_end_flush();

	function requestDetail() {
		ob_start();
		var_dump( $_SERVER );
		var_dump( $_REQUEST );		
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}


	function maxString( $input, $maxlength ) {
	
		if( strlen( $input ) <= $maxlength ) {
			return $input;
		}
		
		return substr( $input, 0, $maxlength );
	
	}



	function unread() {

		$uid = $_SESSION['userid'];

		if( ! is_numeric( $uid ) ) {
			return;
		}

		$sql = "SELECT fileid, filename, min(commentid) as commentid
					FROM offensive_uploads u, offensive_bookmarks b
					WHERE b.userid = $uid AND u.id = b.fileid
					group by fileid
					LIMIT 50";

		$link = openDbConnection();

		$result = mysql_query( $sql );
		
		if( mysql_num_rows( $result ) == 0 ) {
			return;
		}
	
	?>
	
	
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">unread comments:</div>
					<div class="bluebox">
	<?
	
		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = $css == "evenfile" ? "oddfile" : "evenfile";
	?>
			<div class="clipper"><a class="<?= $css ?>" href="?c=comments&fileid=<?= $row['fileid'] ?>#<?= $row['commentid']?>"><?= htmlentities($row['filename']) ?></a></div>
	<?
	
		}
	?>

					</div>

					<div class="heading" style="text-align:center">
						<a class="orange" href="markallread.php">mark all read</a>
					</div>

				<div class="blackbar"></div>
			</div>
	

	<?
	
	}



	
?>
