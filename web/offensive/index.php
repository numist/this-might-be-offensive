<?
	/* if tmbo is down, and you're working on it,
	 *          set this variable to true.
	 * this will send non-admins to index.fixing.php,
	 * helping to  prevent any further possible damage.
	 */
	$fixing = false;
	// now, stay calm, let's do this thing.

/*****************************************************************************/

	/* if tmbo is going down for an upgrade, set this to true and get to work.
	 * this will redirect users to an upgrading page, and will put a notice
	 * at the top of all index pages notifying admins that the site is being
	 * worked on.
	 */
	$upgrading = false;

/*****************************************************************************/

// how many users to display in the online list
$userlimit = 20;
// how long before a user is considered not online anymore
$timelimit = 10;

/*****************************************************************************/

	set_include_path("..");
	require_once("offensive/assets/header.inc");

	time_start($ptime);

	// in an upgrade, break glass:
	if( $upgrading &&
	    (!array_key_exists("status", $_SESSION) ||
	    $_SESSION['status'] != "admin") ) {
		header("Location: ./index.upgrade.php");
		exit;
	}

	// in an emergency, break glass:
	if( $fixing && 
	    (!array_key_exists("status", $_SESSION) ||
	    $_SESSION['status'] != "admin") ) {
		header("Location: ./index.fixing.php");
		exit;
	}

	// Include, and check we've got a connection to the database.
	require_once('admin/mysqlConnectionInfo.inc');
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/functions.inc');

	if( isset( $_REQUEST['c'] ) ) {
		$c = $_REQUEST['c'];
	}
	else {
		$c = (array_key_exists("thumbnails", $_COOKIE) && 
		      $_COOKIE["thumbnails"] == "yes") ? 
		      "thumbs" : "main";
		header("Location: ./?c=$c");
		exit;
	}

	if( ! file_exists( "content/{$c}.inc" ) ) {
		header("Location: ./?c=".(
		       array_key_exists("thumbnails", $_COOKIE) && 
		       $_COOKIE["thumbnails"] == "yes" ? 
		       "thumbs" : "main"));
		exit;
	}

	require( "offensive/content/{$c}.inc" );

	if( function_exists( 'start' ) ) {
		start();
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">


<html>
<head>
	<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
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
	<?
		if(function_exists('head')) {
			echo head();
		}
	?>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<link rel="icon" href="/favicon.ico" />
	<link rel="shortcut icon" href="/favicon.ico" />
	<link rel="stylesheet" type="text/css" href="/offensive/filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>
	<link rel="stylesheet" type="text/css" href="/offensive/nsfw.css.php"/>

	
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

<script type="text/javascript">
	function gochat()
	{
		w = 792;
		h = 646;
		l = (screen.width-w)/2;
		t = (screen.height-h)/2;
		widthHeight = "width="+w+",height="+h+",left="+l+",top="+t+",menubar=no,resizable=yes,scrollbars=no,status=no,toolbar=no,location=no";
		window.open("/offensive/pages/chat.php","tmbo_chat",widthHeight);
		return true
	}
</script>

<?
	if( function_exists( 'head' ) ) {
		head();
	}
?>

</head>

<body bgcolor="#333366" link="#000066" vlink="#000033">

<?php 
	if($upgrading) {
		echo "upgrade in progress.  if you're not doing it, don't touch anything.\n";
	}
	if($fixing) {
		echo "someone's trying to fix the site.  if it's not you, try not to break anything.\n";
	}
	if($readonly) {
		echo "tmbo is currently read-only.  don't try anything funny.\n";
	}

	require( "includes/headerbuttons.txt" );
?>
<br>

	<div id="content">
	
		<div id="titleimg"><a href="./"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></a></div>

	
		<div id="leftcol">

			<?
			// log in --> get info restricted block
			if (array_key_exists("userid", $_SESSION) && is_numeric($_SESSION['userid'])) {
			?>
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

<? /*
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

			<?
			} // log in --> get info restricted block
			?>

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
<!--
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">brought to you by:</div>
					<div class="bluebox" style="text-align:center">
						<a href="http://tengun.net">tengun.net</a>
					</div>
				<div class="blackbar"></div>
			</div>
-->
			<?
			 // archive <--> bottom restricted block
			if(array_key_exists("userid", $_SESSION) && is_numeric($_SESSION["userid"])) {
			?>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="heading">archives:</div>
				<div class="bluebox">
					<?php 
	/* zips */
	$fileList = array();
	$path = "zips";
	$dir = opendir( $path );
	while( ($file = readdir($dir) ) !== false) {
		if( strpos( $file, ".zip" ) !== false ) {
			$fileList[] = $file;
		}
	}

	sort( $fileList );
	$fileList = array_reverse( $fileList );

	foreach( $fileList as $file ) {
		?><a href="/offensive/<?=$path?>/<?php echo $file; ?>"><?php echo $file; ?></a> (<?php echo byte_format(filesize($path . "/" . $file), 1)?>)<br/><?php
	}
					?>
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
					<? /* <a href="audio_rss.php"><img src="graphics/rss_mp3s.gif" border="0" alt="rss: audio" width="77" height="15" style="margin-bottom:6px"></a><br/> */ ?>
					<a href="zip_rss.php"><img src="graphics/rss_zips.gif" border="0" alt="rss: zips" width="77" height="15"></a>
				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">contact:</div>
					<div class="bluebox">
						<a onClick="gochat()">chat</a><br>
						<a href="/contact/">email</a><br>
						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
				<div class="blackbar"></div>
			</div>
			<? if(array_key_exists("c", $_REQUEST) && $_REQUEST['c'] != "online" &&
			      array_key_exists("status", $_SESSION) && $_SESSION['status'] == "admin") { ?>
			<div class="contentbox">
				<div class="blackbar"></div>
					<? 
					if($_REQUEST['c'] != "comments") whosOn(); 
					else subscribed();
					?>
				<div class="blackbar"></div>
			</div>
			<?
			} 
			
		} // archive <--> bottom restricted block
		?>
			
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

	<? require('includes/footer.txt'); ?>

	<div class="textlinks">portions &copy; 1997-<?= date("Y") ?>. site development by <a href="/contact/" class="textlinks" onmouseover='window.status="[ connect ]"; return true' onmouseout='window.status=""'>ray hatfield</a>, scott perry, and others.</div>
	<div class="textlinks"><?= number_format(time_end($ptime), 3)."s php, ".number_format($querytime, 3)."s sql, $queries queries"; ?></div>
</div>
<br />

<? include_once("analytics.inc"); ?>

</body>
</html>
<?
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

		$sql = "SELECT DISTINCT b.fileid, u.filename, b.commentid
					FROM offensive_uploads u, offensive_subscriptions b
					WHERE b.userid = $uid 
						AND u.id = b.fileid
						AND b.commentid IS NOT NULL
					ORDER BY fileid ASC
					LIMIT 50";

		$link = openDbConnection();

		$result = tmbo_query( $sql );
		
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
			$css = isset($css) && $css == "evenfile" ? "oddfile" : "evenfile";
	?>
			<div class="clipper"><a class="<?= $css ?>" href="?c=comments&fileid=<?= $row['fileid'] ?>#<?= $row['commentid']?>"><?= 
			    htmlEscape($row['filename']);
			?></a></div>
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

	function whosOn() {
		global $link, $userlimit, $timelimit;

		if(!isset($link) || !$link) $link = openDbConnection();

		// get the total number of users online
		$sql = "SELECT COUNT(*) FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE)";
		$result = tmbo_query($sql);
		list($nonline) = mysql_fetch_array($result);

		// start us off.
?>		<div class="heading">who's on:</div>
					<div class="bluebox">
						<table style="width:100%">
<?
		
		// list out the latest people to do something
		$sql = "SELECT userid, username FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE) && userid != ".$_SESSION['userid']." ORDER BY timestamp DESC LIMIT $userlimit";
		$result = tmbo_query($sql);
		while(false !== (list($userid, $username) = mysql_fetch_array($result))) {
			$css = (!isset($css) || $css == "odd") ? "even" : "odd";
			echo "<tr class=\"".$css."_row\"><td class=\"".$css."file\"><a href=\"./?c=user&userid=$userid\">$username</a></td></tr>\n";
		}

		$css = (!isset($css) || $css == "odd") ? "even" : "odd";
		// obviously, we're online.
		if($nonline < $userlimit) {
			?><tr class="<?= $css ?>_row"><td class="<?= $css ?>file">you.</td></tr><?
		} else if($nonline > $userlimit) {
			?><tr><td><a href="./?c=online">and <?= ($nonline - $userlimit) ?> more</a></td></tr><?
		}

		echo "\t\t\t\t\t\t</table>\n\t\t\t\t\t</div>\n";
	}

	function subscribed() {
		global $link;

		if(!is_numeric($_REQUEST['fileid'])) trigger_error("non-numeric fileid!", E_USER_ERROR);

		if(!isset($link) || !$link) $link = openDbConnection();

		$sql = "SELECT DISTINCT u.userid, u.username FROM offensive_subscriptions sub JOIN users u ON sub.userid = u.userid WHERE fileid = ".$_REQUEST['fileid']." ORDER BY u.username ASC";
		$result = tmbo_query($sql);

		if(mysql_num_rows($result) == 0) {
			?><div class="heading">no watchers :(</div>
			<?
			return;
		}

		// start us off.
?>		<div class="heading">subscribers:</div>
			<div class="bluebox">
				<table style="width:100%">
<?
		while(false !== (list($userid, $username) = mysql_fetch_array($result))) {
			$css = (!isset($css) || $css == "odd") ? "even" : "odd";
			echo "<tr class=\"".$css."_row\"><td class=\"".$css."file\"><a href=\"./?c=user&userid=$userid\">$username</a></td></tr>\n";
		}

		echo "\t\t\t\t\t\t</table>\n\t\t\t\t\t</div>\n";
	}
	
	record_hit();
?>
