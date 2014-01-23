<?php
	// Include, and check we've got a connection to the database.
	require_once("admin/mysqlConnectionInfo.inc");
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/tabs.inc");
	require_once("offensive/assets/classes.inc");

	function title() {
		global $upload;
		return "[discuss] : " . $upload->filename();
	}

	function start() {
		global $upload, $uploader;
		mustLogIn();
		
		if( ! is_intger( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$upload = core_getupload($_REQUEST['fileid']);
		$uploader = $upload->uploader();

		$upload->clearSubscription();

		// update pickuplink for discussions
		if($upload->type() == "topic") {
			PickupLink::content($upload->type());
			PickupLink::update($upload);
		}

	
	  // check to see if someone's posting.  
	  /* check if someone is being hoodwinked into voting by link.
	   * NB: only works if the client is reporting REFERERS. 
	   */
	  if($upload->type() != 'topic' &&
		 array_key_exists("submit", $_REQUEST) && array_key_exists("HTTP_REFERER", $_SERVER) &&
	     $_SERVER['HTTP_REFERER'] !== "" && strpos($_SERVER['HTTP_REFERER'], $_SERVER['HTTP_HOST']) === false) {
		echo "you've been had.";
		trigger_error("off-site tried to vote on non-topic upload, referer: ".$_SERVER['HTTP_REFERER'], E_USER_WARNING);
		return;
	  }
	  
	  // quietly snub them if they are not an admin and they are trying to post to the changeblog
	  if(array_key_exists("submit", $_REQUEST) && $upload->id() == "211604" && me()->status() != "admin") {
	      return;
	  }
	  
		if( array_key_exists("submit", $_POST)) {
			$args = $_POST;
			if(array_key_exists("offensive", $args) && $args["offensive"] != "") $args["offensive"] = 1;
			if(array_key_exists("repost", $args) && $args["repost"] != "") $args["repost"] = 1;
			if(array_key_exists("subscribe", $args) && $args["subscribe"] != "") $args["subscribe"] = 1;
			$comment = core_postcomment($args);

			if(!is_object($comment)) {
				return;
			}

			if( array_key_exists("redirect", $_REQUEST) ) {
				header("Location: ".Link::upload($_REQUEST['fileid']));
				exit;
			}
			
			// redirecting to the same place prevents 'reload' from reposting the comment.
			header("Location: ".Link::comment($comment));
			exit;
		}
	}

	function head() {
		JS::add("/offensive/js/subscriptions.js");
		JS::add("/offensive/js/favico.js");
	}

	function head_post_js() {
		global $upload;
		?>
		<script type="text/javascript">
			getSocket("<?php $t = new Token("realtime"); echo $t->tokenid(); ?>", function(socket) {
				var latest_comment;

				function isLatestCommentVisible() {
					var docViewTop = $(window).scrollTop();
					var docViewBottom = docViewTop + $(window).height();
					
					var elemTop = $(latest_comment).offset().top;
					var elemBottom = elemTop + $(latest_comment).height();
					
					return ((elemBottom >= docViewTop) && (elemTop <= docViewBottom)
					&& (elemBottom <= docViewBottom) &&  (elemTop >= docViewTop) );
				}

				var favicon = new Favico({
					animation: 'slide',
					bgColor: '#ccc',
					textColor: '#fff'
				});

				var unread = 0;

				function handleEvent() {
					if (latest_comment && isLatestCommentVisible()) {
						latest_comment = null;
						unread = 0;
						favicon.badge(unread);
						$.ajax("/offensive/api.php/resetsubscription.json", {
							data: {
								threadid: '<?= $upload->id() ?>'
							}
						});
					}
					return true;
				}

				$(document).on('mousemove keydown DOMMouseScroll mousewheel mousedown touchstart touchmove', handleEvent);


				socket.on('comment', function(comment) {
					var color = ($("#commentList .entry").length % 2) == 0 ? "#ccccff" : "#bbbbee";
					var html = $(comment.html);
					$(html[2]).css({'background': color});
					if($.inArray(comment.userid, me.squelched) != -1)
						$(html[2]).find('.comment_body').html('<div class="squelched">[ squelched ]</div>');
					if ($('#thumbnail').length > 0)
						$('#thumbnail').before(html);
					else
						$("#commentList").append(html);
					$(html[2]).hide();
					var currentScroll = $(window).scrollTop();
					var slideUp = $('textarea').is(':focus');
					var propTotals = {};
					latest_comment = $(html[2]);
					if(comment.comment) {
						unread += 1;
						favicon.badge(unread);
					}
					$(html[2]).animate({
    				"height": "show",
    				"marginTop": "show",
    				"marginBottom": "show",
    				"paddingTop": "show",
    				"paddingBottom": "show"
					}, {
						step: function(now, fx) {
							if (slideUp) {
								propTotals[fx.prop] = now;
								var scrollTo = currentScroll;
								$.each(propTotals, function(idx, change) { scrollTo += change; });
								$(window).scrollTop(scrollTo);
							}
						}
					});
				});
				socket.emit('subscribe', '/uploads/<?= $upload->id() ?>');
			});
		</script>
		<?php
	}
	
	function toggleThumbnailLink() {
		?><div style="margin:4px;font-weight:bold"><?
			if(me()->getPref("thumbnails_in_comments") == 1) {
				?><a href="<?= Link::setPref("thumbnails_in_comments", 0) ?>">hide thumbnail</a><?
			}
			else {
				?><a href="<?= Link::setPref("thumbnails_in_comments", 1) ?>">show thumbnail</a><?
			}
		?></div><?

	}

	function body() {
		global $upload, $uploader;
	
		$comments_exist = count($upload->getComments()) > 0;
	
		$comments_heading = "the dorks who came before you said:";
		$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:";

		$href = Link::upload($upload);
?>	

<div class="contentbox">
	<div class="blackbar"></div>
		<div class="heading"><?
    // changeblog special header
	if($upload->id() == "211604") {
		?>
		changeblog
		<br/><span style="color:#666699">beat to fit. paint to match.</span><br/>
		<?
	} else if( $upload->type() == 'topic' ) {
		$prefix = $uploader->status() == "admin" ? "" : "don't blame me,";

		echo htmlEscape($upload->filename());

		?><br/><span style="color:#666699"><?= $prefix ?> <?= $uploader->htmlUsername() ?> started it.</span><br/>
<?
	} else {
?>
		<a class="heading" id="pic" href="<?= $href ?>"><?= $upload->htmlFilename() ?></a><br/><span style="color:#666699">uploaded by <?= $uploader->htmlUsername() ?></span><br/>
<?
	}	
?></div>
	
<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div id="commentList" class="bluebox" style="text-align:left">	

<?php 

	$commentnum = 0;
	if( $comments_exist > 0) { 
        // changeblog doesn't say anything demeaning about the people who posted.
        if($upload->id() != "211604") { 
			echo "<b>$comments_heading</b>";
        }

		$numgood = 0;
		$numbad = 0;
		foreach($upload->getComments() as $comment) {
			$css = nextStyle($commentnum++);
			require("offensive/templates/comment.inc");
		}
	
	// end of "are there any comments"
	}
	
		// thumbnail
		if( $upload->type() != 'topic' && ($upload->type() != "audio" || $upload->thumb())) {
			?>
				<div id="thumbnail" style="background:#ccccff; border-top:1px solid #000033">
					<div style="float:right"><? toggleThumbnailLink() ?></div>
					<?
						if(me()->getPref("thumbnails_in_comments") == 1) {
							?>
							
							<a href="<?= $href ?>"
								<? if($upload->filtered()) { ?>
									onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
								<? } ?>
							><img name="th<?= $upload->id()?>" style="margin:12px;"
								src="<?= $upload->filtered()
									? "/offensive/graphics/th-filtered.gif" 
									: $upload->thumbURL() ?>" border="0"
							/></a>
							
							<?
						}
					?>
					
				</div>
				<div style="clear:both">
				</div>
			<?
		}

	?>
	
		

			</div>
			<div class="heading">
				<div style="float:right">					
					<? if( $upload->subscribed() ) { ?>
						<b><a id="subscribeLink" href="subscribe.php?un=1&fileid=<?= $upload->id() ?>" title="take this thread off my 'unread comments' watch list." class="orange">unsubscribe</a></b>
					<? }
						else { ?>
						<b><a id="subscribeLink" href="subscribe.php?fileid=<?= $upload->id() ?>" title="watch this thread for new comments." class="orange">subscribe</a></b>
					<?	}
					?>
				</div>

					<div>
					<? if ($upload->type() != "topic") { ?>
						<a class="heading" id="pic" href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename(); ?></a><br/>
					<? }
						else {
						 ?>&nbsp;<?
						}
					?>
					</div>

			</div>
	</div>

<?php
    // changeblog shouldn't tease the users with a comment field
    if($upload->canComment()) {
?>
<script>
	$(function() {
		$("#comment_form").submit(function(e) {
			e.preventDefault();
			e.stopPropagation();
			var data = {
				vote: $('#comment_form input[name=vote]:checked').val() || "novote",
				comment: $("#comment_form textarea[name=comment]").val() || "",
				tmbo: $("#comment_form #tmbo").attr("checked") ? "1" : "0",
				repost: $("#comment_form #repost").attr("checked") ? "1" : "0",
				subscribe: $("#comment_form #subscribe").attr("checked") ? "1" : "0",
				fileid: $("#comment_form #fileid").val()
			};

			$("#comment_form input:submit").prop('disabled', true);

			var request = $.post('/offensive/api.php/postcomment.php', data);
			request.done(function() {
				if(data.vote != "novote")
					$("#vote_controls").remove();
			  if(data.comment != '' || data.vote == "this is bad" || data.subscribe != "0")
					toggle_subscribe("subscribe", data.fileid, $("#subscribeLink"));
				$("#comment_form")[0].reset();
			});
			request.always(function() {
				$("#comment_form input:submit").prop('disabled', false);
			});
		});
	});
</script>

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading"><?= $add_comment_heading?></div>
			<div class="bluebox" style="text-align:center">			
			<a name="form"></a>
			<form method="POST" id="comment_form" action="<?php echo $_SERVER['PHP_SELF']?>">

			
				<p>
					<input type="hidden" id="fileid" name="fileid" value="<?= $upload->id() ?>"/>
					<input type="hidden" name="c" value="comments"/>
					<textarea name="comment" rows="12" cols="64"></textarea>
				</p>
				


<?php 

	if($upload->canVote()) {
	
	?>	
	    <div id="vote_controls" style="text-align:left;margin-left:10%">
			<input type="radio" name="vote" value="novote" checked="checked" id="novote"/>
			<br/>			
			<input type="radio" name="vote" value="this is good" id="this_is_good"/>
			<label for="this_is_good">[ this is good ]</label><br/>
			<input type="radio" name="vote" value="this is bad" id="this_is_bad"/>
			<label for="this_is_bad">[ this is bad ]</label><br/>
		<br/>

		<input type="checkbox" name="offensive" value="omg" id="tmbo"/>
		<label for="tmbo">[ this might be offensive ]</label><br/>
		
		<input type="checkbox" name="repost" value="police" id="repost"/>
		<label for="repost">[ this is a repost ]</label><br/>

		<br/>

		<input type="checkbox" name="subscribe" value="subscribe" id="subscribe"/>
		<label for="subscribe">[ subscribe ]</label><br/>


</div>

	<?php

	}
?>

						
				<p>
					<input type="submit" name="submit" value="go"/>
				</p>
				
				</form>
			</div>
			<div class="blackbar"></div>
		</div>
			

<?
    }
}

	function nextStyle($i) {
		global $style;
	    $style = $i % 2 ? "background:#bbbbee;" : "background:#ccccff";
		return $style;
	}


?>
