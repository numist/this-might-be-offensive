<?

// how many users to display in the online list
$userlimit = 20;
// how long before a user is considered not online anymore
$timelimit = 10;

/*****************************************************************************/

	$defaultpath = ini_get('include_path');
	set_include_path("..");
	require_once("offensive/assets/header.inc");

	time_start($ptime);

	function query_string($remove = null, $prefix = "") {
		parse_str($_SERVER['QUERY_STRING'], $params);
		foreach(explode(" ", $remove) as $key) {
			if(array_key_exists($key, $params)) {
				unset($params[$key]);
			}
		}
		$ret = http_build_query($params);
		return strlen($ret) > 0 ? $prefix.$ret : $ret;
	}

	// if we're logged in, we'll want access to the user object for the logged in user
	require_once("offensive/assets/classes.inc");

	// in an upgrade, break glass:
	if( $upgrading &&
	    (!array_key_exists("status", $_SESSION) ||
	    $_SESSION['status'] != "admin") ) {
		require("offensive/index.upgrade.php");
		exit;
	}

	// in an emergency, break glass:
	if( $fixing && 
	    (!array_key_exists("status", $_SESSION) ||
	    $_SESSION['status'] != "admin") ) {
		require("offensive/index.fixing.php");
		exit;
	}

	// Include, and check we've got a connection to the database.
	require_once('admin/mysqlConnectionInfo.inc');
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/functions.inc');

	// initialize global $me information if possible.
	login();

	// set our target to any of the requested content page...
	if(isset($_REQUEST['c']) &&
		// if it exists
		file_exists("content/".$_REQUEST["c"].".inc") &&
		// prevent someone from doing something like c="../../../../../etc/passwd"
		strpos($_REQUEST['c'], ".") === false) {
		
		$c = $_REQUEST['c'];
		
	} else {
		// or the default landing page for this session.
		// if not logged in, force it.
		if(!me()) {
			mustLogIn();
		}
		
		$c = (me()->getPref("index") == "thumbs") ? 
		      "thumbs" : "main";
		header("Location: ./?c=$c");
		exit;
	}

	// source the content
	require_once( "offensive/content/$c.inc" );

	if( function_exists( 'start' ) ) {
		start();
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">


<html>
<head>
	<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
	<title><?
		if( function_exists( 'title' ) ) {
			echo title();
		}
		else {
			echo "[ this might be offensive ]";
		}
	?></title>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<link rel="icon" href="/favicon.ico" />
	<link rel="shortcut icon" href="/favicon.ico" />
	<link rel="stylesheet" type="text/css" href="/offensive/filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>

	
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
		window.open("/offensive/irc/chat.php","tmbo_chat",widthHeight);
		return true
	}
	
	/* image rollover stuff */
	function changesrc(a,im)
	{
		x = eval("document."+a);
		x.src=im;
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

			<? if (login()) { // log in --> get info restricted block ?>
				<div class="contentbox">
					<div class="blackbar"></div>
						<div class="heading">your stuff:</div>
						<div class="bluebox">
							<p>hi <b><?= me()->htmlUsername() ?></b>!</p>
							
							<p><a href="index.php?c=upload">upload</a></p>
							
							<p><a href="./?c=subscriptions">subscribed threads</a></p>
							
							<p><a href="./?c=settings">settings</a></p>
            	
							<p><a href="logout.php">log out</a></p>
						</div>
					<div class="blackbar"></div>
				</div>
				<?
					if(function_exists('sidebar')) {
						sidebar();
					}
					
					unread();
				?>
				<div class="contentbox">
					<div class="blackbar"></div>
						<div class="heading">get info:</div>
						<div class="bluebox">
							<p><a href="/offensive/?c=map">maxxer world map</a></p>
							<p><a href="<?= $_SERVER['PHP_SELF'] ?>?c=referral">invite a friend</a></p>						
							<p><a href="<?= $_SERVER['PHP_SELF'] ?>?c=faq">FAQ, rules, etc.</a></p>
							<p><a href="http://tmboradio.com">tmboradio.com</a></p>
							<!-- <p><a href="./?c=stats">stats</a></p> -->						
						</div>
					<div class="blackbar"></div>
				</div>
			<? } // log in --> get info restricted block ?>

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
			<? if(login()) { // archive <--> bottom restricted block ?>
				<div class="contentbox">
					<div class="blackbar"></div>
					<div class="heading">archives:</div>
					<div class="bluebox"><?php 
						/* zips */
						$fileList = array();
						$path = "zips";
						$dir = opendir( $path );
						while( $dir && ($file = readdir($dir) ) !== false)
							if( strpos( $file, ".zip" ) !== false )
								$fileList[] = $file;

						rsort( $fileList );

						foreach( $fileList as $file ) {
							?><a href="/offensive/<?= $path ?>/<?= $file; ?>"><?= $file; ?></a>
							(<?= byte_format(filesize($path . "/" . $file), 1) ?>)<br/><?php
						} ?>
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
						<a href="<?= Link::rss("pic") ?>"><img src="graphics/rss_pics.gif" border="0" alt="rss: pics" width="77" height="15" style="margin-bottom:6px"></a><br/>
						<a href="<?= Link::rss("zip") ?>"><img src="graphics/rss_zips.gif" border="0" alt="rss: zips" width="77" height="15"></a>
					</div>			
					<div class="blackbar"></div>
				</div>

				<div class="contentbox">
					<div class="blackbar"></div>
					<div class="heading">contact:</div>
					<div class="bluebox">
						<a href="#" onClick="gochat(); return false;">chat</a><br>
						<a href="/contact/">email</a><br>
						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
					<div class="blackbar"></div>
				</div>
				<? if($c != "online" && me()->status() == "admin") { ?>
					<div class="contentbox">
						<div class="blackbar"></div>
						<? if($c != "comments") whosOn();
						else whosubscribed(); ?>
						<div class="blackbar"></div>
					</div>
				<? } 
			} // archive <--> bottom restricted block ?>
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

	<div class="textlinks">portions &copy; 1997-<?= date("Y") ?>.
		site development by
		<a href="/contact/" class="textlinks" onmouseover='window.status="[ connect ]"; return true' onmouseout='window.status=""'>ray hatfield</a>,
		<a href="mailto:thismightbe@numist.net">scott perry</a>,
		and others.</div>
	<div class="textlinks">tmbo runs on
		<a href="http://www.imagemagick.org" title="for messing with images">ImageMagick</a>,
		<a href="http://xapian.org" title="for comment search">Xapian</a>,
		and unicorn farts.
	</div>
	<?
	
	if(me()->status() == "admin") {
		?>
		<div class="textlinks"><?= number_format(time_end($ptime), 3)."s php, ".number_format($querytime, 3)."s sql, ".count($queries)." queries\n\n <!--\n\n";
			var_dump($queries);
			echo "\n\n-->\n\n"; ?></div>
		<?
		$loadavg = "/proc/loadavg";
		if(file_exists($loadavg) && is_readable($loadavg)) {
			$load = file_get_contents($loadavg);
			?>
			<div class="textlinks"><?= $load ?></div>
			<?
		}
	}
	
	?>
</div>
<br />

<? include_once("analytics.inc"); ?>

</body>
</html>
<?
	// XXX: this needs to use core
	function unread() {
		if(!me()) return;
		$uid = me()->id();

		$sql = "SELECT DISTINCT u.*, b.commentid
					FROM offensive_uploads u, offensive_subscriptions b
					WHERE b.userid = $uid 
						AND u.id = b.fileid
						AND b.commentid IS NOT NULL
					ORDER BY b.commentid ASC
					LIMIT 50";

		$result = tmbo_query( $sql );
		
		if( mysql_num_rows( $result ) == 0 ) {
			$hidden = "none";
		} else {
			$hidden = "block";
		} ?>
		
		<div id="unread" class="contentbox" style="display: <?= $hidden ?>;">
			<div class="blackbar"></div>
			<div class="heading">unread comments:</div>
			<div class="bluebox">
				<? while( $row = mysql_fetch_assoc( $result ) ) {
					$css = isset($css) && $css == "evenfile" ? "oddfile" : "evenfile"; 
					$upload = new Upload($row); ?>
				
					<div class="clipper"><a class="<?= $css ?>" href="?c=comments&fileid=<?= $upload->id() ?>#<?= $row['commentid']?>"><?= $upload->htmlFilename() ?></a></div>
				<? } ?>
			</div>
			<div class="heading" style="text-align:center">
				<a class="orange" href="markallread.php">mark all read</a>
			</div>
			<div class="blackbar"></div>
		</div>
	<? }

	// XXX: this needs to use core
	function whosOn() {
		global $userlimit, $timelimit;

		// get the total number of users online
		$sql = "SELECT COUNT(*) FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE)";
		$result = tmbo_query($sql);
		list($nonline) = mysql_fetch_array($result);

		// start us off. ?>
		<div class="heading">who's on:</div>
		<div class="bluebox">
			<table style="width:100%"><?
				$uid = me() ? me()->id() : 0;
				// list out the latest people to do something
				$sql = "SELECT userid, username FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE) && userid != $uid ORDER BY timestamp DESC LIMIT $userlimit";
				$result = tmbo_query($sql);
				while(false !== (list($userid, $username) = mysql_fetch_array($result))) {
					$css = (!isset($css) || $css == "odd") ? "even" : "odd"; ?>
					<tr class="<?= $css ?>_row"><td class="<?= $css ?>file"><a href="./?c=user&userid=<?= $userid ?>"><?= $username ?></a></td></tr>
				<? }

				$css = (!isset($css) || $css == "odd") ? "even" : "odd";
				// obviously, we're online.
				if($nonline < $userlimit) {
					?><tr class="<?= $css ?>_row"><td class="<?= $css ?>file">you.</td></tr><?
				} else if($nonline > $userlimit) {
					?><tr><td><a href="./?c=online">and <?= ($nonline - $userlimit) ?> more</a></td></tr><?
				} ?>
			</table>
		</div>
	<? }

	function whosubscribed() {
		if(!is_intger($_REQUEST['fileid'])) trigger_error("non-numeric fileid!", E_USER_ERROR);

		$sql = "SELECT DISTINCT u.userid, u.username FROM offensive_subscriptions sub JOIN users u ON sub.userid = u.userid WHERE fileid = ".$_REQUEST['fileid']." ORDER BY u.username ASC";
		$result = tmbo_query($sql);

		if(mysql_num_rows($result) == 0) { ?>
			<div class="heading">no watchers :(</div>
			<? return;
		}

		// start us off. ?>
		<div class="heading">subscribers:</div>
		<div class="bluebox">
			<table style="width:100%">
				<? while(false !== (list($userid, $username) = mysql_fetch_array($result))) {
					$css = (!isset($css) || $css == "odd") ? "even" : "odd"; ?>
					<tr class="<?= $css?>_row"><td class="<?= $css ?>file"><a href="./?c=user&userid=<?= $userid ?>"><?= $username ?></a></td></tr>
				<? } ?>
			</table>
		</div>
	<? } ?>
