<?php 
	set_include_path("../..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/assets/functions.inc");
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/comments.inc");

	mustLogIn();
	
	$me = new User($_SESSION["userid"]);

	if(array_key_exists("random", $_REQUEST)) {
		header("Location: /offensive/pages/pic.php?id=".get_random_id());
		exit;
	}
	
	$id = "";
	if(array_key_exists("id", $_REQUEST)) {
		$id = $_REQUEST["id"];
	}
	
	if(!is_numeric($id)) {
		header( "Location: /offensive/" );
		exit;
	}

	$upload = new Upload($id);
	if($upload->type() == "topic") {
		header("Location: /offensive/?c=comments&fileid=".$id);
		exit;
	}

	if($upload->type() == "image") {
		// update the pickup cookie
		$cookiename = $me->id()."lastpic";
		if(!array_key_exists($cookiename, $_COOKIE) ||
		   !is_numeric($_COOKIE[$cookiename]) ||
		   $_COOKIE[$cookiename] < $upload->id()) {
			setcookie( $cookiename, $upload->id(), time() + 3600*24*365*10, "/offensive/");
			$cookiepic = $upload->id();
		} else {
			$cookiepic = $_COOKIE[$cookiename];
		}
				
		// update the pickup db entry
		if($me->getPref("ipickup") == false || $me->getPref("ipickup") < $upload->id()) {
			$me->setPref("ipickup", $upload->id());
		}
	}

	function get_random_id() {
		global $me;
		
		$cookiename = $me->id()."lastpic";
		if(array_key_exists($cookiename, $_COOKIE)) {
			$cookiepic = $_COOKIE[$cookiename];
		} else {
			// this should never happen, since in normal browsing you have to hit
			// pic.php at least once with an id argument in order to use random.
			$cookiepic = 0;
		}
		
		/*
		 * since pic.php sets the ipickup db preference and the pickupid in the cookie
		 * at the beginning of execution if they are invalid, it's safe to skip
		 * existence and type checks at this point.
		 */
		$sql = "SELECT id FROM offensive_uploads WHERE type='image' AND status='normal' AND id < ".min($me->getPref('ipickup'), $cookiepic)." ORDER BY RAND() LIMIT 1";
		$res = tmbo_query($sql);
		$row = mysql_fetch_assoc( $res );
		return($row['id']);
	}

	function getFileSize( $fpath ) {
		$k = "";
		if( file_exists( $fpath ) ) {
			$size = filesize( $fpath );
			$k = byte_format($size);
		}
		return $k;
	}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
   "http://www.w3.org/TR/html4/loose.dtd">

<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>[<?= $upload->type() ?>] : <?= $upload->filename() ?> </title>
		<link rel="stylesheet" type="text/css" href="styles.php"/>
		<script type="text/javascript" src="/offensive/js/jquery-1.2.6.min.js"></script>
		<!-- XXX: a lot of this picui stuff is going to have to move into this header so it can be customized -->
		<script type="text/javascript" src="/offensive/js/picui.js"></script>
		<script type="text/javascript" src="/offensive/js/subscriptions.js"></script>
		<script type="text/javascript" src="/offensive/js/jqModal.js"></script>
		<script type="text/javascript" src="/offensive/js/jqDnR.js"></script>
		<script type="text/javascript">
			self.file_id = "";
			
			// prevent sites from hosting this page in a frame;
			if( window != top ) {
				top.location.href = window.location.href;
			}
			
			// handle a keybased event. this code was incorporated from offensive.js, which has now been deprecated
			function handle_keypress(o,e)
			{
				var id;
				if(e == null)  return true;

				var keycode = (e.which == null) ? e.keyCode : e.which;
				switch( keycode ) {
					<?
					require("offensive/data/keynav.inc");
					// get the user's keyboard navigation preferences
					$prefs = array();
					foreach($options as $option => $foo) {
						if($option == "noselect") continue;
						$val = $me->getPref($option);
						if($val) {
							$prefs[$option] = unserialize($val);
						}
					}
			    	
					if(count($prefs) == 0) {
						// use the default keybindings, the user has set nothing special.
						?>
						case 61:  // +
						case 107: // + (numpad)
						case 187: // =
						case 174: // Wii +
							e.preventDefault();
							id = $("#good");
							if(id.parent().hasClass('on')) {
								do_vote(id);
							}
							return;
							break;
                	
						case 109: // - (numpad)
						case 189: // -
						case 170: // Wii -
							e.preventDefault();
							id = $("#bad");
							if(id.parent().hasClass('on')) {
								do_vote(id);
							}
							return;
							break;
                	
						case 81:  // q
							e.preventDefault();
							$("#dialog").jqmShow();
							return;
							break;
                	
						case 191: // ?
							e.preventDefault();
							document.location.href = "/offensive/pages/pic.php?random";
							return;
							break;
                	
					// following not ajaxified
						case 39:  // →
						case 177: // Wii Right
							e.preventDefault();
							id = "previous";
							break;
                	
						case 37:  // ←
						case 178: // Wii Left
							e.preventDefault();
							id = "next";
							break;
                	
						case 38:  // ↑
						case 175: // Wii Up
							e.preventDefault();
							id = "index";
							break;
                	
						case 40:  // ↓
						case 176: // Wii Down
							e.preventDefault();
							id = "comments";
							break;
						<?
						$escape = array("27");
					} else {
						if(array_key_exists("key_good", $prefs)) {
							foreach($prefs["key_good"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = $("#good");
							if(id.parent().hasClass('on')) {
								do_vote(id);
							}
							return;
							break;
							<?
						}
						
						if(array_key_exists("key_bad", $prefs)) {
							foreach($prefs["key_bad"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = $("#bad");
							if(id.parent().hasClass('on')) {
								do_vote(id);
							}
							return;
							break;
							<?
						}
                	
						if(array_key_exists("key_quick", $prefs)) {
							foreach($prefs["key_quick"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							$("#dialog").jqmShow();
							return;
							break;
							<?
						}
						
						if(array_key_exists("key_random", $prefs)) {
							foreach($prefs["key_random"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							document.location.href = "/offensive/pages/pic.php?random";
							return;
							break;
							<?
						}
						
						if(array_key_exists("key_prev", $prefs)) {
							foreach($prefs["key_prev"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = "previous";
							break;
							<?
						}
						
						if(array_key_exists("key_next", $prefs)) {
							foreach($prefs["key_next"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = "next";
							break;
							<?
						}
						
						if(array_key_exists("key_index", $prefs)) {
							foreach($prefs["key_index"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = "index";
							break;
							<?
						}
						
						if(array_key_exists("key_comments", $prefs)) {
							foreach($prefs["key_comments"] as $code) {
								echo "case $code:\n";
							}
							?>
							e.preventDefault();
							id = "comments";
							break;
							<?
						}

						if(array_key_exists("key_subscribe", $prefs)) {
							foreach($prefs["key_subscribe"] as $code) {
								echo "case $code:\n";
							}
							?>
							sub=$('.subscribe_toggle:visible');
							handle_subscribe(sub,e,$("#good").attr("name"));
							return;
							break;
							<?
						}

						if(array_key_exists("key_escape", $prefs)) {
							$escape = $prefs["key_escape"];
						} else {
							$escape = array();
						}
					}
					?>
				}
				if( id && document.getElementById( id ) ) {
					document.location.href = document.getElementById( id ).href;
					return false;
				}
				return true;
			}

			// handle a keybased event. this code was incorporated from offensive.js, which has now been deprecated
			function handle_qc_keypress(o,e)
			{
				if(e.metaKey || e.altKey || e.shiftKey || e.ctrlKey) return true;

				var id;
				if(e == null)  {
					return true;
				}

				var keycode = (e.which == null) ? e.keyCode : e.which;
				switch( keycode ) {
					<?
					foreach($escape as $keycode) {
						echo "case $keycode:\n";
					}
					if(count($escape) > 0) { ?>
						e.preventDefault();
						$("#dialog").jqmHide();
						return;
						break;
					<? } ?>
				}
				return true;
			}

		</script>
	</head>
	<body>
		<!-- message -->
		<div style="white-space:nowrap;overflow:hidden;padding:3px;margin-bottom:0px;background:#000033;color:#ff6600;font-size:10px;font-weight:bold;padding-left:4px;">
			<? if(count($prefs) == 0) { ?>
				<div id="instruction_link" style="float:right;"><a href="#" style="color:#ff6600">?</a></div>
			<? } ?>
			<div>consciousness doesn't really exist. it's just another one of our ideas.</div>
		</div>
		<? if(count($prefs) == 0) { ?>
			<div id="instructions" style="display:none;white-space:nowrap;overflow:hidden;padding:3px;margin-bottom:6px;background:#cccccc;color:#333333">
				keyboard commands:<br />
				← = newer. ↑ = index. → = older. ↓ = comments . + or = votes [ this is good ]. - votes [ this is bad ] .<br />
				q = quick comment, Esc closes quick comment box, ? = random image.<br />
				( change 'em at your <a href="/offensive/?c=settings">settings</a> page. )
			</div>
		<? } ?>
    	
		<!-- this window is not visible unless you do a quick comment -->
		<!-- data is fetched using ajax in js and put in #qc_bluebox  -->
		<div class="jqmWindow" id="dialog">
			<div class="blackbar"></div>
			<div class="heading"><table style="width: 100%;"><tr>
				<td align="left">let's hear it</td>
				<td class="qc_close" align="right"><a href="#" class="jqmClose">Close</a></td>
			</tr></table></div>
			<div class="bluebox" id="qc_bluebox" style="text-align: center">
			</div>
		</div> <!-- end quickcomment -->
		
		<div id="content">
			<div id="heading" style="white-space:nowrap;">
				&nbsp;&nbsp;
				<?
				/*
				 * navigation buttons, prev index next are dependant on type
				 */
				if($upload->type() == 'avatar') {?>
					<a href="../" id="next" style="visibility:hidden">newer</a> . <a id="index" href="/offensive/">index</a> . <a id="previous" href="../" style="visibility:hidden">older</a>
					<?
				} else {
					if($upload->next()) { ?>
						<a id="next" href="<?= $_SERVER['PHP_SELF'] ?>?id=<?= $upload->next()->id() ?>">newer</a>
					<? } else { ?>
						<a href="../" id="next" style="visibility:hidden">newer</a>
					<? } ?>
					. <a id="index" href="/offensive/">index</a> .
					<? if($upload->prev()) {?>
						<a id="previous" href="<?= $_SERVER['PHP_SELF'] ?>?id=<?= $upload->prev()->id() ?>">older</a>
					<? } else { ?>
						<a id="previous" href="../" style="visibility:hidden">older</a>
					<?}
				} ?>
					
				<!--
					comment block
				-->
				<a style="margin-left:48px;"
				   id="comments"
				   href="/offensive/?c=comments&fileid=<?= $upload->id() ?>">comments</a>
				(<span id="count_comment"><?= $upload->comments() ?></span>c
				+<span id="count_good"><?= $upload->goods() ?></span>
				-<span id="count_bad"><?= $upload->bads() ?></span><?
				if($upload->tmbos() > 0) { ?>
					<span style=\"color:#990000\">x<?= $upload->tmbos() ?></span>
				<? } ?>)
				&nbsp;(<a id="quickcomment" class="jqModal" href="#">quick</a>)
    	
				<!--
					voting block
				-->
				<span style="margin-left:40px;">
					<?
					if(canVote($upload->id()) && $upload->file()) {
						$good_href = "href=\"/offensive/?c=comments&submit=submit&fileid=$id&vote=this%20is%20good&redirect=true\"";	
						$bad_href = "href=\"/offensive/?c=comments&submit=submit&fileid=$id&vote=this%20is%20bad&redirect=true\"";
						$class = "on";
					} else {
						$good_href = $bad_href = "";
						$class = "off";
					}
						
					?>
					<span id="votelinks" class="<?= $class ?>">
						vote: <a name="<?= $upload->id() ?>" id="good" class="votelink" <?= $good_href ?>>[ this is good ]</a> .
						<a name="<?= $upload->id() ?>" id="bad" class="votelink" <?= $bad_href ?>>[ this is bad ]</a>
					</span>
				</span>
    	
				<!--
					subscribe block
				-->
				<span style="margin-left:48px;">
					<?	
					if(subscribed($upload->id())) { ?>
						<a class="subscribe_toggle" id="unsubscribeLink" href="/offensive/subscribe.php?un=1&fileid=<?= $id ?>" title="take this file off my 'unread comments' watch list.">unsubscribe</a>
					<?	} else { ?>
						<a class="subscribe_toggle" id="subscribeLink" href="/offensive/subscribe.php?fileid=<?= $id ?>" title="watch this thread for new comments.">subscribe</a>
					<?	} ?>
				</span>

	    		<!--
	    		    filter block
	    		-->
	    		<span style="margin-left:48px;">filters:</span>
				<span style="margin-left:5px;"><?
	    		        if($me->getPref("hide_nsfw") == 1) { ?>
	    		                <a href="/offensive/setPref.php?p=hide_nsfw&v=">nsfw(on)</a>
	    		        <? } else { ?>
	    		                <a href="/offensive/setPref.php?p=hide_nsfw&v=1">nsfw(off)</a>
	    		        <? } ?>
	    		</span>
            	
	    		<span style="margin-left:5px;"><?
	    		        if($me->getPref("hide_tmbo") == 1) { ?>
	    		                        <a href="/offensive/setPref.php?p=hide_tmbo&v=">tmbo(on)</a>
	    		        <? } else { ?>
	    		                        <a href="/offensive/setPref.php?p=hide_tmbo&v=1">tmbo(off)</a>
	    		        <? } ?>
	    		</span>
			</div>
	
			<br />



			<!--
				filename/size block
			-->
			<br />
			<?
				if($upload->is_nsfw()) { ?>
					<a style="color:#990000;" href="/offensive/setPref.php?p=hide_nsfw&v=<?= $me->getPref("hide_nsfw") == 1 ? "" : "1" ?>" title="<?= $me->getPref("hide_nsfw") == 1 ? "show" : "hide" ?> images that are not safe for work">[nsfw]</a><?
				}
				if($upload->is_tmbo()) { ?>
					<a style="color:#990000;" href="/offensive/setPref.php?p=hide_tmbo&v=<?= $me->getPref("hide_tmbo") == 1 ? "" : "1" ?>" title="<?= $me->getPref("hide_tmbo") == 1 ? "show" : "hide" ?> images that might be offensive">[tmbo]</a><?
				}
				
				$style = ($upload->is_tmbo() || $upload->is_nsfw()) ? "style=\"margin-left:.3em\"" : "";
				
				echo "<span $style>" . htmlEscape($upload->filename()) . "</span>" ;
			?>
			<span style="color:#999999"><? 
				if($upload->file() != "")
					echo getFileSize($upload->file());
			?></span>
			<br/>
				
			<!--
				username/time block
			-->
			<span style="color:#999999">
				uploaded by <a id="userLink" href="../?c=user&userid=<?= $upload->uploader()->id() ?>"><?= $upload->uploader()->username() ?></a> @ <?= $upload->timestamp() ?>
			</span>
			
			<!--
				squelch block
			-->
			<span style="margin-left:48px">
				<?
				if($me->squelched($upload->uploader()->id())) {
					?><a id="unsquelchLink" style="color:#999999; text-decoration:underline" href="/offensive/setPref.php?unsq=<?= $upload->uploader()->id() ?>">unsquelch <?= $upload->uploader()->username() ?></a><?
				} else {
					?><a id="squelchLink" style="color:#999999; text-decoration:underline" href="/offensive/setPref.php?sq=<?= $upload->uploader()->id() ?>">squelch <?= $upload->uploader()->username() ?></a><?
				}
				?>
			</span>
			<br/><br/>
			
			<!--
				image block
			-->
			<? if($me->squelched($upload->uploader()) || 
			    ($upload->is_nsfw() == 1 && $me->getPref("hide_nsfw") == 1) || 
			    ($upload->is_tmbo() == 1 && $me->getPref("hide_tmbo") == 1)) {
				?><div style="padding:128px;">[ <a id="imageLink" href="<?= $upload->URL() ?>" target="_blank">filtered</a>:<?
					if($me->squelched($upload->uploader())) {
						?> squelched <!-- <?= $upload->uploader()->id() ?> --><?
					}
					if($upload->is_nsfw() == 1 && $me->getPref("hide_nsfw") == 1) {
						echo " nsfw";
					}
					if($upload->is_tmbo() == 1 && $me->getPref("hide_tmbo") == 1) {
						echo " tmbo";
					}
				?> ]</div><?
			} else { ?>
				<div class="<?php echo $upload->is_nsfw() == 1 ? 'nsfw' : 'image' ?> u<?= $upload->uploader()->id() ?>">
					<? if($upload->file() != "") { ?>
						<a id="imageLink" href="<?= $upload->URL() ?>" target="_blank"><img src="<?= $upload->URL() ?>" style="border:none"/></a>
					<? } else { ?>
						<div style="padding:128px;">[ got nothin' for ya ]</div>
					<? } ?>
				</div>
			<? } ?>
			<br/><br/>
		</div>
    	
		<br />&nbsp;
    	
		<? record_hit();
		include_once("analytics.inc"); ?>
	</body>
</html>
