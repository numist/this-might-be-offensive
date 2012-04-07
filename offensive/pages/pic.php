<?php
	set_include_path("../..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/assets/functions.inc");
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	require_once("offensive/classes/assets.inc");

	mustLogIn();
	time_start($ptime);

	$id = "";
	if(array_key_exists("id", $_REQUEST)) {
		$id = $_REQUEST["id"];
	}

	if(!is_intger($id)) {
		header( "Location: /offensive/" );
		exit;
	}

	$upload = core_getupload($id);
	if(!$upload->exists()) {
		header( "Location: /offensive/" );
		exit;
	}
	PickupLink::content($upload->type());

	if(array_key_exists("random", $_REQUEST)) {
		header("Location: ".Link::upload(get_random_id($upload)));
		exit;
	}

	if($upload->type() == "topic") {
		header("Location: ".Link::thread($id));
		exit;
	}

	###########################################################################
	// update pickuplinks
	global $autoplay;

	$autoplay = PickupLink::update($upload);

	if(array_key_exists('loop', $_REQUEST)) {
		$autoplay = true;
	}

	###########################################################################
	function get_random_id($upload) {
		$pickuplinks = PickupLink::get();

		$filter = me()->getPref("hide_nsfw") ? " AND nsfw = 0" : "";
		$filter .= me()->getPref("hide_tmbo") ? " AND tmbo = 0" : "";
		$sql = "SELECT id FROM offensive_uploads WHERE type='".$upload->type()."' AND status='normal' AND id < ".min($pickuplinks).$filter." ORDER BY RAND() LIMIT 1";
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
	###########################################################################
?>
<!DOCTYPE HTML>

<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
		<META NAME="ROBOTS" CONTENT="NOARCHIVE" />
		<title>[<?= $upload->type() ?>] : <?= $upload->filename() ?> </title>
		<!-- <? if($upload->next_filtered()) { ?>
			<link rel="prefetch" href="<?= $_SERVER['PHP_SELF'] ?>?id=<?= $upload->next_filtered()->id() ?>"/>
		<? } ?> -->

		<script type="text/javascript">
			// handle a keybased event. this code was incorporated from offensive.js, which has now been deprecated
			function handle_keypress(e)
			{
				// keycode translation
	      <? require("offensive/data/keynav.inc"); ?>
	      function composite_keycode(e)
	      {
	        var keycode = (e.which == null) ? e.keyCode : e.which;
					if(e.shiftKey) {
	    		  keycode |= <?= KEY_SHIFT ?>;
	    		}
	    		if(e.altKey) {
	    		  keycode |= <?= KEY_ALT ?>;
	    		}
	    		if(e.ctrlKey) {
	    		  keycode |= <?= KEY_CTRL ?>;
	    		}
	    		if(e.metaKey) {
	    		  keycode |= <?= KEY_META ?>;
	    		}
	    		// TODO: remove key-agnosticism
	    		keycode |= <?= KEY_META_AWARE ?>;

	    		return keycode;
	      }

			  // potential actions
			  function nav_to_id(id) {
          if(document.getElementById(id)) {
  					document.location.href = document.getElementById(id).href;
  				}
        }
        
      	function key_next()     { nav_to_id("next"); };
      	function key_prev()     { nav_to_id("previous"); };
      	function key_comments() { nav_to_id("comments"); };
      	function key_index()    { nav_to_id("index"); };
        function key_good() { do_vote($("#good")); };
      	function key_bad()  { do_vote($("#bad")); };
      	function key_quick() { $("#qc_dialog").dialog("open"); };
      	function key_subscribe() { handle_subscribe($('.subscribe_toggle:visible'),e,$("#good").attr("name")); };
        function key_random() { document.location.href = "<?= Link::upload($upload) ?>&random"; };
        function key_image_toggle() { theimage().irsz("toggle"); };
      	
				if(e == null)  return true;
        var keycode = composite_keycode(e);

        <?// get the user's keyboard navigation preferences
				$prefs = array();
				foreach($key_options as $option => $foo) {
					if($option == "noselect") continue;
					$val = me()->getPref($option);
					if($val) {
						$prefs[$option] = unserialize($val);
					}
				}
				if(count($prefs) == 0) {
				  $prefs = $key_defaults;
			  }
        
        foreach($prefs as $action => $codes) {
          foreach($codes as $code) {
            // TODO: remove key-agnosticism
            if($code <= KEY_CODE_MASK) {
              // code is modifier-agnostic ?>
            if(<?= $code ?> == (keycode & <?= KEY_CODE_MASK ?>)) {
            <? } else {
              // code is modifier-strict ?>
            if(<?= $code ?> == keycode) {
            <? } ?>
              
              e.preventDefault();
              <?= $action ?>();
              return false;
            }
        
        <?}
        }?>

				return true;
			}
		</script>
		<script type="text/javascript" src="/socket.io/socket.io.js"></script>
<?
	CSS::add("/styles/jquery-ui-1.8.17.custom.css");
	CSS::add("/styles/pic.css");
	CSS::emit();
	JS::add("/offensive/js/jquery-1.7.1.min.js");
	JS::add("/offensive/js/tmbolib.js");
	JS::add("/offensive/js/jquery-ui-1.8.17.custom.min.js");
	JS::add("/offensive/js/jquery.ba-outside-events.min.js");
	JS::add("/offensive/js/subscriptions.js");
	JS::add("/offensive/js/irsz.js");
	JS::add("/offensive/js/picui.js");
	JS::add("/offensive/js/analytics.js");
	JS::emit();
?>
		<script type="text/javascript">
			var me = {
				hide_nsfw: <?= me()->getPref("hide_nsfw") == 1 ? 'true' : 'false' ?>,
				hide_tmbo: <?= me()->getPref("hide_tmbo") == 1 ? 'true' : 'false' ?>,
				hide_bad: <?= me()->getPref("hide_bad") == 1 ? 'true' : 'false' ?>,
				squelched: <?= json_encode(me()->squelched_list()) ?>
			}
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				socket.on('comment', function(comment) {
					var stats_row = $("#voting_stats");
					if (comment.comment) {
						var comments_count = parseInt(stats_row.find('#count_comment').text()) + 1;
						stats_row.find('#count_comment').text(comments_count);
					}
					if (comment.vote) {
						var replace = comment.vote == 'this is good' ? '#count_good' : '#count_bad';
						var vote_count = parseInt(stats_row.find(replace).text()) + 1;
						stats_row.find(replace).text(vote_count);
					}
					if (comment.tmbo) {
						var tmbo_span = stats_row.find('#count_tmbo');
						if (tmbo_span.length == 0) {
							stats_row.find("#count_bad").after(' x<span id="count_tmbo" style="color:#990000">0</span>');
							tmbo_span = stats_row.find('#count_tmbo');
						}
						var tmbo_count = parseInt(tmbo_span.text()) + 1;
						tmbo_span.text(tmbo_count);
					}
				});
				socket.emit('subscribe', '/uploads/<?= $upload->id() ?>');
			});

		</script>
	</head>
	<body id="pic">
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
				( change 'em in your <a href="<?= Link::content("settings") ?>">settings</a>. )
			</div>
		<? } ?>
		
		<!-- quick comment box -->
		<div id="qc_dialog" style="display: none;">
			<a name="form"></a>
			<form id="qc_form"<? if(!$upload->canComment()) { ?> style="display: none;"<? } ?>>
					<input type="hidden" value="329310" name="fileid" id="qc_fileid">
					<input type="hidden" name="c" value="comments">
					<textarea cols="64" rows="6" name="comment" id="qc_comment"></textarea>

					<? if($upload->canVote()) { ?>
						<div id="qc_vote" style="text-align:left;margin-left:14%">
							<table><tbody><tr><td width="200px">
							<input class="qc_tigtib" id="qc_novote" type="radio" value="novote" name="vote" checked="">
							<br>
          	
							<input class="qc_tigtib" type="radio" name="vote" value="this is good" id="qc_tig">
							<label for="qc_tig">[ this is good ]</label><br>
          	
							<input class="qc_tigtib" type="radio" name="vote" value="this is bad" id="qc_tib">
							<label for="qc_tib">[ this is bad ]</label><br>
							</td>
							<td>
							<input type="checkbox" name="offensive" value="omg" id="qc_tmbo">
							<label for="qc_tmbo">[ this might be offensive ]</label><br>
          	
							<input type="checkbox" name="repost" value="police" id="qc_repost">
							<label for="qc_repost">[ this is a repost ]</label><br>
							<input type="checkbox" name="subscribe" value="subscribe" id="qc_subscribe">
							<label for="qc_subscribe">[ subscribe ]</label><br>
							</td></tr></tbody></table>
          	
						</div>
					<? } ?>
					<div id="qc_go" style="text-align: center">
						<input type="submit" name="submit" value="go">
					</div>
			</form>
			<div id="qc_comments">
			  <div id="qc_commentrows">
			  </div>
			</div>
		</div>

		<div id="content">
			<div id="heading" style="white-space:nowrap;">
				&nbsp;&nbsp;<span id="navigation_controls">
				<?
				/*
				 * navigation buttons, prev index next are dependant on type
				 */
				$index="";
				switch($upload->type()) {
					case "avatar":
						$index = "yearbook";
						break;
					case "audio":
						$index = "audio";
						break;
					default:
						$index = me()->getPref("index");
						break;
				}

				if($upload->next_filtered()) {
					$style = ($upload->next_filtered()->is_nsfw() || $upload->next_filtered()->is_tmbo() ? 'style="font-style:italic; color: #990000"' : "") ?>
					<a id="next" <?= $style ?> href="<?= Link::upload($upload->next_filtered()) ?>" title="<?= str_replace('"', '\\"', $upload->next_filtered()->filename()) ?>">newer</a>
				<? } else { ?>
					<a href="<?= Link::content($index) ?>" id="next" style="visibility:hidden">newer</a>
				<? } ?>
				. <a id="index" href="<?= Link::content($index) ?>">index</a> .
				<? if($upload->prev_filtered()) {
					$style = ($upload->prev_filtered()->is_nsfw() || $upload->prev_filtered()->is_tmbo() ? 'style="font-style:italic; color: #990000"' : "") ?>
					<a id="previous" <?= $style ?> href="<?= Link::upload($upload->prev_filtered()) ?>" title="<?= str_replace('"', '\\"', $upload->prev_filtered()->filename()) ?>">older</a>
				<? } else { ?>
					<a id="previous" href="<?= Link::content($index) ?>" style="visibility:hidden">older</a>
				<? } ?>
				</span>

				<!--
					comment block
				-->
				<span id="voting_stats">
					<a style="margin-left:48px;"
					   id="comments"
					   href="<?= Link::thread($upload) ?>">comments</a>
					(<span id="count_comment"><?= $upload->comments() ?></span>c
					+<span id="count_good"><?= $upload->goods() ?></span>
					-<span id="count_bad"><?= $upload->bads() ?></span><?
					if($upload->tmbos() > 0) { ?>
						x<span id="count_tmbo" style="color:#990000"><?= $upload->tmbos() ?></span>
					<? } ?>)
					<span id="quicklink">&nbsp;(<a id="quickcomment" href="#">quick</a>)</span>
				</span>

				<!--
					voting block
				-->
				<span id="voting_controls" style="margin-left:40px;">
					<?
					if($upload->canVote()) {
						// TODO: clean up
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
				<span id="subscribe" style="margin-left:48px;">
					<?
					if($upload->subscribed()) { ?>
						<a class="subscribe_toggle" id="unsubscribeLink" href="/offensive/subscribe.php?un=1&fileid=<?= $id ?>" title="take this file off my 'unread comments' watch list.">unsubscribe</a>
					<?	} else { ?>
						<a class="subscribe_toggle" id="subscribeLink" href="/offensive/subscribe.php?fileid=<?= $id ?>" title="watch this thread for new comments.">subscribe</a>
					<?	} ?>
				</span>

				<!--
				    filter block
				-->
				<span id="filter_controls">
					<span style="margin-left:48px;">filters:</span>
					<span style="margin-left:5px;"><?
					        if(me()->getPref("hide_nsfw") == 1) { ?>
					                <a href="<?= Link::setPref("hide_nsfw", "") ?>">nsfw(on)</a>
					        <? } else { ?>
					                <a href="<?= Link::setPref("hide_nsfw", 1) ?>">nsfw(off)</a>
					        <? } ?>
					</span>
        	
					<span style="margin-left:5px;"><?
					        if(me()->getPref("hide_tmbo") == 1) { ?>
					                        <a href="<?= Link::setPref("hide_tmbo", "") ?>">tmbo(on)</a>
					        <? } else { ?>
					                        <a href="<?= Link::setPref("hide_tmbo", 1) ?>">tmbo(off)</a>
					        <? } ?>
					</span>
				</span>
			</div>

			<br />

			<!--
				filename/size block
			-->
			<br />
			<?
				if($upload->is_nsfw()) { ?>
					<a style="color:#990000;" href="<?= Link::setPref("hide_nsfw", (me()->getPref("hide_nsfw") == 1 ? "" : "1")) ?>" title="<?= me()->getPref("hide_nsfw") == 1 ? "show" : "hide" ?> images that are not safe for work">[nsfw]</a><?
				}
				if($upload->is_tmbo()) { ?>
					<a style="color:#990000;" href="<?= Link::setPref("hide_tmbo", (me()->getPref("hide_tmbo") == 1 ? "" : "1")) ?>" title="<?= me()->getPref("hide_tmbo") == 1 ? "show" : "hide" ?> images that might be offensive">[tmbo]</a><?
				}

				$style = ($upload->is_tmbo() || $upload->is_nsfw()) ? "style=\"margin-left:.3em\"" : "";

				echo "<a href=\"".$upload->URL()."\" target=\"_blank\"><span $style>" . htmlEscape($upload->filename()) . "</span></a>";

			?>
			<span id="dimensions" style="color:#999999"><?
				if($upload->file() != "")
					echo getFileSize($upload->file());
			?></span>
			<br/>

			<!--
				username/time block
			-->
			<span style="color:#999999">
				uploaded by <?= $upload->uploader()->htmlUsername() ?> @ <?= $upload->timestamp() ?>
			</span>
			<br/><br/>
			
			<!--
				file block
			-->
			<? 

			if($upload->type() == "audio") {
				require_once("offensive/assets/id3.inc");
				
				$args = "mp3=".urlencode($upload->URL())."&amp;".
						"width=500&amp;".
						"showvolume=1&amp;".
						"showloading=always&amp;".
						"buttonwidth=25&amp;".
						"sliderwidth=15&amp;".
						"volumewidth=36&amp;".
						"volumeheight=8&amp;".
						"loadingcolor=9d9d9d&amp;".
						"sliderovercolor=9999ff&amp;".
						"buttonovercolor=9999ff";

				// if the upload is filtered, do not automatically play
				// likewise, if we've seen this before and are not asking to loop, do not autoplay
				if($autoplay) {
					$args .= "&amp;autoplay=1";
				}
				if(array_key_exists('loop', $_REQUEST)) {
					$args .= "&amp;loop=1";
				}

				if(file_exists($upload->file())) {
					$fp = fopen($upload->file(), 'r');
					$id3 = new getid3_id3v2($fp, $info);
					?><table><tr><td height="100px" width="100px" align="right"><?
					if(array_key_exists('id3v2', $info) && array_key_exists('comments', $info['id3v2'])) {
						if(file_exists($upload->thumb())) {
							?>
							<a href="/offensive/ui/albumArt.php?id=<?= $upload->id() ?>"
								<? if($upload->filtered()) { ?>
									onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
								<? } ?> target="_blank"
							><img name="th<?= $upload->id()?>"
								src="<?= $upload->filtered()
									? "/offensive/graphics/th-filtered.gif" 
									: $upload->thumbURL() ?>"
								alt="album art" border="0"
							/></a>
							<?
						}

						?></td><td><?

						$tags = $info['id3v2']['comments'];

						if(array_key_exists('title', $tags)) { ?>
						<span style="color:#666666">Title: <?= trim($tags['title'][0]); ?>
							<?
							if(array_key_exists('tracknum', $tags)) {
								echo "(track ".(int)trim($tags['tracknum'][0]);
								if(array_key_exists('totaltracks', $tags)) {
									echo " of ".(int)trim($tags['totaltracks'][0]);
								}
								echo ")";
							}
							?>
						</span><br />
						<? }

						if(array_key_exists('artist', $tags)) { ?>
						<span style="color:#666666">By: <?= trim($tags['artist'][0]); ?></span><br />
						<? }

						if(array_key_exists('album', $tags)) { ?>
						<span style="color:#666666">Album: <?= trim($tags['album'][0]); ?></span><br /><br />
						<? }
					}
					?></td></tr></table><?

					?>

						<!--<audio src="<?= $upload->URL() ?>" controls<?
							if($autoplay) {
								echo " autoplay";
							}
							if(array_key_exists('loop', $_REQUEST)) {
								echo " loop";
							}
						?>> -->
						<object type="application/x-shockwave-flash" data="/offensive/ui/player_mp3_maxi.swf" width="500" height="20">
							<param value="transparent" name="wmode" />
							<param name="movie" value="/offensive/ui/player_mp3_maxi.swf" />
							<param name="bgcolor" value="#ffffff" />
							<param name="FlashVars" value="<?= $args ?>" />
						</object>
					<!--</audio>-->

					<table><tr><td style="text-align:right" width="480px">
							&nbsp;
							<? if(!array_key_exists('loop', $_REQUEST)) { ?>
									<a style="color:#999999; text-decoration:underline" href="<?= Link::upload($upload) ?>&loop">loop</a>
							<? } ?>
					</td></tr></table>
					<?
				} else { ?>
					<div style="padding:128px;">[ got nothin' for ya ]</div><?
				}
			} else if($upload->type() == "image" || $upload->type() == "avatar") {
				if( $upload->filtered() ) {
					?><div style="padding:128px;">[ <a id="imageLink" href="<?= $upload->URL() ?>" target="_blank">filtered</a>:<?
						if($upload->squelched()) {
							echo " squelched <!-- ".$upload->uploader()->id()
							     ." - ".$upload->uploader()->username()." -->";
						}
						if($upload->filtered_nsfw()) {
							echo " nsfw";
						}
						if($upload->filtered_tmbo()) {
							echo " tmbo";
						}
						if($upload->filtered_bad()) {
							echo " bad";
						}
					?> ]</div><?
				} else { ?>
					<div class="<?php echo $upload->is_nsfw() == 1 ? 'nsfw' : 'image' ?> u<?= $upload->uploader()->id() ?>">
						<? if($upload->file() != "") {
							$dimensions = $upload->dimensions(); ?>
							<a id="imageLink" href="<?= $upload->URL() ?>"><img src="<?= $upload->URL() ?>" style="border:none" id="image"
								 max-width="<?= $dimensions[0] ?>px" max-height="<?= $dimensions[1] ?>px"/></a>
						<? } else { ?>
							<div style="padding:128px;">[ got nothin' for ya ]</div>
						<? } ?>
					</div>
					<?
				} 
			} ?>
		</div>
    	
		<?
		if(me()->status() == "admin") {
			?>
			<!--
			page stats block
			-->
			<br />
		
			<center><div style="color:#ccc;"><?= number_format(time_end($ptime), 3)."s php, ".number_format($querytime, 3)."s sql, ".count($queries)." queries\n\n <!--\n\n";
				var_dump($queries);
				echo "\n\n-->\n\n"; ?></div>
			<?
			$loadavg = "/proc/loadavg";
			if(file_exists($loadavg) && is_readable($loadavg)) {
				$load = file_get_contents($loadavg);
				?>
				<div style="color:#ccc;"><?= $load ?></div>
				<?
			}
?>			</center>
			<?
		}
		?>
	</body>
</html>
