<?

// how many users to display in the online list
$userlimit = 20;
// how long before a user is considered not online anymore
$timelimit = 10;

//$downtime = 1327204800; // Sat Jan 21 22:00:00 CST 2012
//$downtime_link = "https://thismight.be/offensive/?c=comments&fileid=327976";
/*****************************************************************************/

	$defaultpath = ini_get('include_path');
	set_include_path("..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/classes/assets.inc");

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

	// attempt session/cookie login.
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
		
		header("Location: ".Link::mainpage());
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
	<script type="text/javascript" src="/socket.io/socket.io.js"></script>
	<?
	CSS::add("/styles/filepilestyle.css");
	CSS::add("/styles/oldskool.css");
	CSS::add("/styles/index.css");
	JS::add("/offensive/js/jquery-1.7.1.min.js");
	JS::add("/offensive/js/tmbolib.js");
	if(function_exists('head')) {
		head();
	}
	JS::add("/offensive/js/analytics.js");
	CSS::emit();
	JS::emit();
	if(function_exists('head_post_js')) {
		head_post_js();
	}
?>
<script type="text/javascript">
	var me = {
		hide_nsfw: <?= me()->getPref("hide_nsfw") == 1 ? 'true' : 'false' ?>,
		hide_tmbo: <?= me()->getPref("hide_tmbo") == 1 ? 'true' : 'false' ?>,
		hide_bad: <?= me()->getPref("hide_bad") == 1 ? 'true' : 'false' ?>,
		squelched: <?= json_encode(me()->squelched_list()) ?>
	}

	$(function() {
		getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
			socket.on('reset_subscription', function(upload) {
				var existing = $('#unread' + upload.id);
				if (existing.length > 0) {
					existing.remove();
					if ($("#unread-container a").length === 0)
						$("#unread").hide();
				}
			});

			socket.on('subscription', function(comment) {
				var link = '/offensive/?c=comments&fileid=' + comment.fileid + '#' + comment.id;
				var existing = $('#unread' + comment.fileid);
				if (existing.length > 0) {
					existing.attr('href', link);
				} else {
					var element = $('<div class="clipper" />');
					var anchor = $('<a id="unread' + comment.fileid + '" href="' + link + '"></a>').text(comment.filename);
					element.append(anchor);
					var prev_id = 0;
					var fileid_int = parseInt(comment.fileid);
					var inserted = false;
					$("#unread-container a").each(function(idx, el) {
						var this_id = parseInt($(el).attr('id').match(/\d+/)[0]);
						if (this_id > fileid_int && prev_id < fileid_int && !inserted) {
							element.insertBefore($(el).parent());
							anchor.addClass(idx % 2 == 0 ? 'evenfile' : 'oddfile');
							inserted = true;
						}
						if (inserted) {
							$(el).addClass(idx % 2 != 0 ? 'evenfile' : 'oddfile').removeClass(idx % 2 == 0 ? 'evenfile' : 'oddfile');
						}
						prev_id = this_id;
					});
					if (element.parent().length === 0) {
						$("#unread-container").append(element);
					  anchor.addClass(($("#unread-container a").length % 2 != 0 ? 'evenfile' : 'oddfile'));
					}
					$("#unread").show();
				}
			});
		});
	});
</script>
</head>

<body bgcolor="#333366" link="#000066" vlink="#000033">

<?php 
	if($upgrading) {
		echo "upgrade in progress.  if you're not doing it, don't touch anything.\n";
	}
	if($fixing) {
		echo "someone's trying to fix the site.  if it's not you, try not to break anything.\n";
	}
	if(TMBO::readonly()) {
		echo "tmbo is currently read-only.  don't try anything funny.\n";
	}

?>
<br>

	<div id="content">
	
		<div id="titleimg"><a href="<?= Link::mainpage() ?>"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></a></div>

	
		<div id="leftcol">

			<? if (login()) { // log in --> get info restricted block ?>
				<div class="contentbox">
					<div class="blackbar"></div>
						<div class="heading">your stuff:</div>
						<div class="bluebox">
							<p>hi <b><?= me()->htmlUsername() ?></b>!</p>
							
							<p><a href="<?= Link::content("upload") ?>">upload</a></p>
							
							<p><a href="<?= Link::content("subscriptions") ?>">subscribed threads</a></p>
							
							<p><a href="<?= Link::content("settings") ?>">settings</a></p>
            	
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
						<div class="heading">community:</div>
						<div class="bluebox">
							<p><a href="http://chat.efnet.org:9090/?channels=themaxx&nick=<?= urlencode(me()->username()); ?>" target="_blank">chat</a></p>
							<p><a href="<?= Link::content("map") ?>">maxxer world map</a></p>
							<p><a href="<?= Link::content("referral") ?>">invite a friend</a></p>						
							<p><a href="<?= Link::content("faq") ?>">FAQ, rules, etc.</a></p>
							<!-- <p><a href="<?= Link::content("stats") ?>">stats</a></p> -->						
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
							<a href="<?= Link::content("ppsub") ?>"><img src="graphics/paypal_subscribe.gif" border="0"/></a>
						</div>
					</div>
				<div class="blackbar"></div>
			</div>
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
							<a class="orange" href="<?= Link::content("hof") ?>">hall of fame</a>
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
						<a href="/contact/">email</a><br>
						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
					<div class="blackbar"></div>
				</div><?
					if($c != "comments" && $c != "online" && me()->status() == "admin") { whosOn(); }
					else if($c == "comments") {
						if(!array_key_exists("fileid", $_REQUEST)
						|| !is_intger($_REQUEST['fileid']))
						{ trigger_error("non-numeric fileid!", E_USER_ERROR); }
						$upload = core_getupload($_REQUEST['fileid']);
						if($upload->uploader()->id() == me()->id() || me()->status() == "admin")
						{ whosubscribed($upload); }
					}
			} // archive <--> bottom restricted block ?>
		</div> <!-- end left column -->
		
		<div id="rightcol">
		
		<?
		if(isset($downtime)) {
		  $left = $downtime - time();
		  if($left >= 0 && $left < 14400) { // 4h
		    $message = "tmbo is going ";
		    if(isset($downtime_link)) $message .= "<a href=\"$downtime_link\">";
		    $message .= "down for maintenance";
		    if(isset($downtime_link)) $message .= "</a>";
		    
		    if($left > 7200) { // 2h
		      $message .= " in ".(int)($left / 3600)." hours";
		    } else if($left > 120) { // 2m
		      $message .= " in ".(int)($left / 60)." minutes";
		    } else {
		      $message .= " SOON";
		    }
		    box($message, "maintenance time!");
	    }
		} ?>

					<?
						if( function_exists( 'body' ) ) {
							body();
						}
					?>
		</div>

	</div>

<br clear="all">
<div class="textlinks" style="text-align:center">

	<? require('includes/footer.txt'); ?>

	<div class="textlinks">portions &copy; 1997-<?= date("Y") ?>.
		site development by
		<a href="/contact/" class="textlinks" onmouseover='window.status="[ connect ]"; return true' onmouseout='window.status=""'>ray hatfield</a>,
		<a href="mailto:thismightbe@numist.net">scott perry</a>,
		and <a href="https://github.com/numist/this-might-be-offensive/contributors">others</a>.</div>
	<div class="textlinks" style="margin: 1em;">Ingredients:
		<a href="http://php.net/" title="to talk to the computer">PHP</a>,
		<a href="http://www.mysql.com/" title="for things of importance">MySQL</a>,
		<a href="http://redis.io/" title="for things that need to be fast">Redis</a>,
		<a href="https://github.com/nrk/predis" title="to get to things">Predis</a>,
		<a href="http://jquery.com/" title="for fancy dynamic things">jQuery</a>,
		<a href="http://flash-mp3-player.net/players/maxi/" title="for making music">MP3 Player</a>,
		<a href="http://www.imagemagick.org" title="for messing with images">ImageMagick</a>,
		<a href="http://xapian.org" title="for comment search">Xapian</a>,
		natural flavor.<br />
		Not a significant source of calcium, iron, dietary fiber, vitamin D, and sanity.
	</div>
	<?
	
	if(me()->status() == "admin") {
		?>
		<div class="textlinks"><?= number_format(time_end($ptime), 3)."s php, ".number_format($querytime, 3)."s sql, ".count($queries)." queries\n\n <!-- query statistics: \n";
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
</body>
</html>
<?
	// XXX: this needs to use core
	function unread() {
		if(!me()) return;
		$uid = me()->id();
		
		$comments = core_unreadcomments(array());

		if(count($comments) == 0) {
			$hidden = "none";
		} else {
			$hidden = "block";
		} ?>
		
		<div id="unread" class="contentbox" style="display: <?= $hidden ?>;">
			<div class="blackbar"></div>
			<div class="heading">unread comments:</div>
			<div id="unread-container" class="bluebox">
				<? foreach ($comments as $comment) {
					$upload = $comment->upload();
					if($upload->squelched()) continue;

					$css = isset($css) && $css == "evenfile" ? "oddfile" : "evenfile"; 
					// XXX: rejigger the query and use Link::comment ?>
					<div class="clipper"><a id="unread<?= $comment->upload()->id()?>" class="<?= $css ?>" href="<?= Link::comment($comment) ?>"><?= $upload->htmlFilename() ?></a></div>
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

		// XXX: is it any faster to combine this and the next query?
		// get the total number of users online
		$sql = "SELECT COUNT(*) FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE)";
		$result = tmbo_query($sql);
		list($nonline) = mysql_fetch_array($result);

		// start us off. ?>
	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading">who's on:</div>
		<div class="bluebox">
			<table style="width:100%"><?
				$uid = me() ? me()->id() : 0;
				// list out the latest people to do something
				$sql = "SELECT * FROM users WHERE timestamp > DATE_SUB( now( ) , INTERVAL $timelimit MINUTE) && userid != $uid ORDER BY timestamp DESC LIMIT $userlimit";
				$result = tmbo_query($sql);
				while(false !== ($row = mysql_fetch_array($result))) {
					$css = (!isset($css) || $css == "odd") ? "even" : "odd"; ?>
					<tr class="<?= $css ?>_row"><td class="<?= $css ?>file"><?= id(new User($row))->htmlUsername() ?></td></tr>
				<? }

				$css = (!isset($css) || $css == "odd") ? "even" : "odd";
				// obviously, we're online.
				if($nonline < $userlimit) {
					?><tr class="<?= $css ?>_row"><td class="<?= $css ?>file">you.</td></tr><?
				} else if($nonline > $userlimit) {
					?><tr><td><a href="<?= Link::content("online") ?>">and <?= ($nonline - $userlimit) ?> more</a></td></tr><?
				} ?>
			</table>
		</div>
		<div class="blackbar"></div>
	</div>
	<? }

	function whosubscribed($upload) {
		$sql = "SELECT DISTINCT u.* FROM offensive_subscriptions sub JOIN users u ON sub.userid = u.userid WHERE fileid = ".$upload->id()." ORDER BY u.username ASC";
		$result = tmbo_query($sql);
		
		$watchers = array();
		while(false !== ($row = mysql_fetch_array($result))) {
			$watcher = new User($row);
			if(!me()->squelched($watcher)) {
				$watchers[] = $watcher;
			}
		}

		if(count($watchers) == 0) { ?>
			<div class="heading">no watchers :(</div>
			<? return;
		}

		// start us off. ?>
	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading">subscribers:</div>
		<div class="bluebox">
			<table style="width:100%">
				<? foreach($watchers as $user) {
					$css = (!isset($css) || $css == "odd") ? "even" : "odd"; ?>
					<tr class="<?= $css?>_row"><td class="<?= $css ?>file"><?= $user->htmlUsername() ?></td></tr>
				<? } ?>
			</table>
		</div>
		<div class="blackbar"></div>
	</div>
	<? } ?>
