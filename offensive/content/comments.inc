<?php
	// Include, and check we've got a connection to the database.
	require_once("admin/mysqlConnectionInfo.inc");
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/tabs.inc");
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/picContext.inc");

	function title() {
		global $upload;
		return "[discuss] : " . $upload->filename();
	}

	function start() {
		global $upload, $uploader, $context;
		mustLogIn();
		
		if( ! is_intger( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$upload = core_getupload($_REQUEST['fileid']);
		$uploader = $upload->uploader();

		// the list the pic page was browsing (picContext.inc), kept on the
		// links back to it
		$context = $upload->type() == "topic" ? null : PicContext::fromRequest($upload);

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
				header("Location: ".($context ? $context->link($upload) : Link::upload($upload)));
				exit;
			}
			
			// redirecting to the same place prevents 'reload' from reposting the comment.
			header("Location: ".($context
				? $context->threadLink($upload)."#".$comment->id()
				: Link::comment($comment)));
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
				var threadid = <?= $upload->id() ?>;
				var latest_comment;

				// the whole of the newest comment that came in live is on screen.
				function isLatestCommentVisible() {
					if(!latest_comment.isConnected)
						return false;
					var rect = latest_comment.getBoundingClientRect();
					return rect.top >= 0 && rect.bottom <= window.innerHeight;
				}

				var favicon = new Favico({
					animation: 'slide',
					bgColor: '#000033',
					textColor: '#ccccff'
				});

				var unread = 0;

				function handleEvent() {
					if (latest_comment && isLatestCommentVisible()) {
						latest_comment = null;
						unread = 0;
						favicon.badge(unread);
						fetch("/offensive/api.php/resetsubscription.json?threadid=" + threadid);
					}
				}

				['mousemove', 'keydown', 'wheel', 'mousedown', 'touchstart', 'touchmove'].forEach(function(type) {
					document.addEventListener(type, handleEvent, { passive: true });
				});

				function commentExists(comment) {
					// comment.inc puts the comment id on the .entry element itself.
					return document.getElementById(String(comment.id)) !== null;
				}

				// a comment from the realtime socket or the api: add it to the list
				// and slide it open. when you're typing in the comment box below
				// the list, the page scrolls along so the box stays put. false if
				// it was already there.
				function addComment(comment) {
					if(commentExists(comment))
						return false;
					// the newest comment on the page, so a later catch-up starts after it.
					lastcomment = Math.max(lastcomment, Number(comment.id));
					var template = document.createElement('template');
					template.innerHTML = comment.html;
					// Find the entry by class: the template's top-level node layout has changed before.
					var entry = template.content.querySelector('.entry');
					if(entry && me.squelched.indexOf(comment.userid) != -1)
						entry.querySelector('.comment_body').innerHTML = '<div class="squelched">[ squelched ]</div>';
					var thumbnail = document.getElementById('thumbnail');
					if(thumbnail)
						thumbnail.before(template.content);
					else
						document.getElementById('commentList').append(template.content);
					if(entry) {
						latest_comment = entry;
						revealComment(entry);
					}
					return true;
				}

				// slides a new comment open from nothing. the overflow it needs while
				// it grows comes off again at the end: left on, it would make the
				// comment the scroll container of its sticky author line (theme.css
				// .comment_meta), which then sits a line or two up for good.
				function revealComment(entry) {
					if(matchMedia('(prefers-reduced-motion: reduce)').matches)
						return;
					var slideUp = document.activeElement && document.activeElement.tagName == 'TEXTAREA';
					var style = getComputedStyle(entry);
					var open = {
						height: style.height,
						paddingTop: style.paddingTop,
						paddingBottom: style.paddingBottom
					};
					var closed = { height: '0px', paddingTop: '0px', paddingBottom: '0px' };
					entry.style.overflow = 'clip';
					var animation = entry.animate([closed, open], { duration: 400, easing: 'ease-in-out' });

					var shown = 0;
					function follow() {
						var height = entry.getBoundingClientRect().height;
						window.scrollBy(0, height - shown);
						shown = height;
					}
					function step() {
						if(animation.playState != 'running')
							return;
						follow();
						requestAnimationFrame(step);
					}
					if(slideUp)
						requestAnimationFrame(step);

					animation.finished.finally(function() {
						entry.style.overflow = '';
						if(slideUp)
							follow();
					});
				}

				// the comments after the newest one on the page, oldest first.
				function fetchNewComments() {
					var query = new URLSearchParams({
						thread: threadid,
						idmin: lastcomment + 1,
						sort: 'date_asc',
						html: 1
					});
					return fetch('/offensive/api.php/getcomments.json?' + query).then(function(response) {
						if(!response.ok)
							throw new Error('getcomments: ' + response.status);
						return response.json();
					});
				}

				// post without leaving the page. the new comment is fetched back
				// rather than waited for on the socket: a socket can look connected
				// and still be dead (a phone that slept, a dropped proxy connection),
				// and then the comment never showed up. the fetch also brings in
				// anything the socket missed, in order.
				var form = document.getElementById('comment_form');
				if(form) form.addEventListener('submit', function(e) {
					e.preventDefault();
					function flag(id) {
						var input = form.querySelector('#' + id);
						return input && input.checked ? "1" : "0";
					}
					var vote = form.querySelector('input[name=vote]:checked');
					var data = new URLSearchParams({
						vote: vote ? vote.value : "novote",
						comment: form.elements.comment.value,
						offensive: flag('tmbo'),
						repost: flag('repost'),
						subscribe: flag('subscribe'),
						fileid: form.elements.fileid.value
					});

					var submit = form.querySelector('input[type=submit]');
					submit.disabled = true;

					fetch('/offensive/api.php/postcomment.json', { method: 'POST', body: data }).then(function(response) {
						if(!response.ok)
							throw new Error('postcomment: ' + response.status);
						if(data.get('vote') != "novote") {
							var voteControls = document.getElementById('vote_controls');
							if(voteControls)
								voteControls.remove();
						}
						if(data.get('comment') != '' || data.get('vote') == "this is bad" || data.get('subscribe') != "0")
							toggle_subscribe("subscribe", data.get('fileid'), $("#subscribeLink"));
						form.reset();
						return fetchNewComments().then(function(comments) {
							comments.forEach(addComment);
						});
					}).catch(function(error) {
						// the text stays in the box, so it can be sent again.
						console.error(error);
					}).finally(function() {
						submit.disabled = false;
					});
				});

				var heldComments = [];
				var handleRealtimeComment = function(comment) {
					heldComments.push(comment);
				}

				socket.on('comment', function(comment) {
					handleRealtimeComment(comment);
				});

				socket.emit('subscribe', '/uploads/' + threadid);

				// whatever came in between rendering the page and the socket connecting.
				fetchNewComments().then(function(comments) {
					comments.forEach(addComment);
				}).catch(function(error) {
					console.error(error);
				}).then(function() {
					heldComments.forEach(addComment);
					handleRealtimeComment = function(comment) {
						// your own comment is usually on the page already.
						if(addComment(comment) && comment.comment) {
							unread += 1;
							favicon.badge(unread);
						}
					}
				});
			});
		</script>
		<?php
	}
	
	function toggleThumbnailLink() {
		?><div style="margin:4px;font-weight:bold"><?php
			if(me()->getPref("thumbnails_in_comments") == 1) {
				?><a href="<?= Link::setPref("thumbnails_in_comments", 0) ?>">hide thumbnail</a><?php
			}
			else {
				?><a href="<?= Link::setPref("thumbnails_in_comments", 1) ?>">show thumbnail</a><?php
			}
		?></div><?php

	}

	// comments with no text, only a vote or a tmbo/repost flag, grouped into
	// runs of consecutive ones. returns the runs that are long enough to
	// collapse (the collapse_votes_min pref), keyed by the index of their
	// first comment.
	function voteOnlyRuns($comments)
	{
		$min = collapseVotesMin();
		$runs = array();
		$start = null;
		foreach($comments as $i => $comment) {
			if(strlen($comment->text())) {
				$start = null;
				continue;
			}
			if($start === null) {
				$start = $i;
				$runs[$start] = array();
			}
			$runs[$start][] = $comment;
		}
		return array_filter($runs, function($run) use ($min) {
			return count($run) >= $min;
		});
	}

	// the row that stands in for a run of vote-only comments while "collapse
	// votes" is on: their totals, written like the counts in the lists, as a
	// button that opens a list of who voted how. it's always in the page, and
	// theme.css shows either it or the comments.
	function collapsedVotes($run)
	{
		$goods = $bads = $tmbos = 0;
		foreach($run as $comment) {
			$goods += $comment->vote() == "this is good" ? 1 : 0;
			$bads += $comment->vote() == "this is bad" ? 1 : 0;
			$tmbos += $comment->tmbo() ? 1 : 0;
		}
		$id = "votes".$run[0]->id();
		?><div class="entry"><button type="button" popovertarget="<?= $id ?>"><span class="goods"><?= $goods ?></span><span class="bads"><?= $bads ?></span><?php
			if($tmbos > 0) { ?><span class="tmbos"><?= $tmbos ?></span><?php }
		?> <span>(votes collapsed)</span></button><ul id="<?= $id ?>" popover><?php
			// who voted, the same way the comments show it. each links to
			// its comment, which shows it even while collapsed (theme.css)
			foreach($run as $comment) {
				$vote = $comment->vote();
				?><li><?= $comment->commenter()->htmlUsername() ?> <a class="comment_votes" href="#<?= $comment->id() ?>"><?php
					if($vote) { ?><span class="vote <?= $vote == "this is good" ? "good" : "bad" ?>"><span>[ <?= $vote ?> ]</span></span><?php }
					if($comment->tmbo()) { ?><span class="vote tmbo"><span>[ this might be offensive ]</span></span><?php }
					if($comment->tiar()) { ?><span class="vote tiar"><span>[ this is a repost ]</span></span><?php }
				?></a></li><?php
			}
		?></ul></div>
<?php
	}

	function body() {
		global $upload, $uploader, $context;
	
		$comments_exist = count($upload->getComments()) > 0;
	
		$comments_heading = "the dorks who came before you said:";
		$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:";

		$href = $context ? $context->link($upload) : Link::upload($upload);
?>	

<div class="contentbox">
	<div class="heading"><?php
	// changeblog special header
	if($upload->id() == "211604") {
	?>
	changeblog
	<br/><span style="color:var(--link-user)">beat to fit. paint to match.</span><br/>
	<?php
	} else if( $upload->type() == 'topic' ) {
	$prefix = $uploader->status() == "admin" ? "" : "don't blame me,";

	echo htmlEscape($upload->filename());

	?><br/><span class="blame"><?= $prefix ?> <?= $uploader->htmlUsername() ?> started it.</span><br/>
<?php
	} else {
?>
		<a class="heading" href="<?= $href ?>"><?= $upload->htmlFilename() ?></a><br/><span class="blame">uploaded by <?= $uploader->htmlUsername() ?></span><br/>
<?php
	}	
?></div>
	
<?php
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div id="commentList" class="bluebox" style="text-align:left">	

<?php 

	$lastcomment = 0;
	if( $comments_exist > 0) { 
		$comments = $upload->getComments();
		$voteRuns = voteOnlyRuns($comments);

	// changeblog doesn't say anything demeaning about the people who posted.
	if($upload->id() != "211604") { 
			// one element, so it doesn't throw off the .entry:nth-child striping
			?><div><b><?= $comments_heading ?></b><label><input type="checkbox" id="link_previews" name="link_previews" aria-label="show link previews" /><span>show</span><span>hide</span> link previews</label><?php
			if(count($voteRuns)) {
				?><label><input type="checkbox" id="collapse_votes" name="collapse_votes" aria-label="collapse votes"<?= me()->getPref("collapse_votes") == 1 ? " checked" : "" ?> /><span>collapse</span><span>expand</span> votes</label><?php
			}
			?></div><?php
		}

		$numgood = 0;
		$numbad = 0;
		// comment.inc marks the comments in a run with data-collapsible
		$collapsible = false;
		foreach($comments as $i => $comment) {
			if(isset($voteRuns[$i])) {
				collapsedVotes($voteRuns[$i]);
				$collapsible = true;
			}
			if(strlen($comment->text())) {
				$collapsible = false;
			}
			$lastcomment = $comment->id();
			require("offensive/templates/comment.inc");
		}
	// end of "are there any comments"
	}
	
		// thumbnail
		if( $upload->type() != 'topic' && ($upload->type() != "audio" || $upload->thumb())) {
			?>
				<div id="thumbnail" style="background:var(--bg-content); border-top:1px solid var(--bg-heading)">
					<div style="float:right"><?php toggleThumbnailLink() ?></div>
					<?php
						if(me()->getPref("thumbnails_in_comments") == 1) {
							?>
							
							<a href="<?= $href ?>"
								<?php if($upload->filtered()) { ?>
									onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
							 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
								<?php } ?>
							><img name="th<?= $upload->id()?>" style="margin:12px;"
								src="<?= $upload->filtered()
									? "/offensive/graphics/th-filtered.gif" 
									: $upload->thumbURL() ?>" border="0"
							/></a>
							
							<?php
						}
					?>
					
				</div>
				<div style="clear:both">
				</div>
			<?php
		}

	?>
	
		

			</div>
			<?php // outside #commentList so it doesn't throw off the .entry:nth-child striping ?>
			<script>
				lastcomment = <?php echo $lastcomment ?>;
			</script>
			<div class="heading">
				<div style="float:right">					
					<?php if( $upload->subscribed() ) { ?>
						<b><a id="subscribeLink" href="subscribe.php?un=1&fileid=<?= $upload->id() ?>" title="take this thread off my 'unread comments' watch list." class="orange">unsubscribe</a></b>
					<?php }
						else { ?>
						<b><a id="subscribeLink" href="subscribe.php?fileid=<?= $upload->id() ?>" title="watch this thread for new comments." class="orange">subscribe</a></b>
					<?php 	}
					?>
				</div>

					<div>
					<?php if ($upload->type() != "topic") { ?>
						<a class="heading" href="<?= $href ?>"><?= $upload->htmlFilename(); ?></a><br/>
					<?php }
						else {
						 ?>&nbsp;<?php
						}
					?>
					</div>

			</div>
	</div>

<?php
	// changeblog shouldn't tease the users with a comment field
	if($upload->canComment()) {
?>

	<div class="contentbox">
		<div class="heading"><?= $add_comment_heading?></div>
		<div class="bluebox" style="text-align:center">			
		<a name="form"></a>
		<form method="POST" id="comment_form" action="<?php echo $_SERVER['SCRIPT_NAME']?>">

			
			<p>
				<input type="hidden" id="fileid" name="fileid" value="<?= $upload->id() ?>"/>
				<input type="hidden" name="c" value="comments"/>
				<?= $context ? $context->hiddenFields() : "" ?>
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
		</div>
			

<?php
	}
}
?>
