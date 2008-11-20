<?php
	// Include, and check we've got a connection to the database.
	require_once("admin/mysqlConnectionInfo.inc");
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/tabs.inc");
	require_once("offensive/assets/comments.inc");
	require_once("offensive/assets/classes.inc");

	function additionalCssFor($vote) {
		if(strpos($vote, 'bad') > 0) return " bad";
		if(strpos($vote, 'good') > 0) return " good";
		return "";
	}

	function title() {
		global $upload;
		return "[discuss] : " . $upload->filename();
	}

	function start() {
		global $upload, $uploader;
	
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);

		if( ! is_numeric( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$upload = new Upload($_REQUEST['fileid']);
		$uploader = $upload->uploader();

		if(!$upload->exists()) {
			require("offensive/404.php");
			exit;
		}

		clearSubscription($upload->id(), $me->id());

		// update pickuplink for discussions
	    if($upload->type() == "topic") {
	    	// update the pickup cookie
        	$cookiename = $me->id()."lasttopic";
        	if(!array_key_exists($cookiename, $_COOKIE) ||
        	   !is_numeric($_COOKIE[$cookiename]) ||
        	   $_COOKIE[$cookiename] < $upload->id()) {
        		setcookie( $cookiename, $upload->id(), time() + 3600*24*365*10, "/offensive/"); 
			}

			// update the pickup db entry
			if($me->getPref("tpickup") == false || $me->getPref("tpickup") < $upload->id()) {
				$me->setPref("tpickup", $upload->id());
			}
        }

	
	    // check to see if someone's posting.  
	    /* check if someone is being hoodwinked into voting by link.
	     * NB: only works if the client is reporting REFERERS. 
	     */
	    if($upload->type() != 'topic' &&
		   array_key_exists("submit", $_REQUEST) && array_key_exists("HTTP_REFERER", $_SERVER) && 
	       $_SERVER['HTTP_REFERER'] !== "" && strpos($_SERVER['SERVER_NAME'], $_SERVER['HTTP_REFERER']) === false) {
			echo "you've been had.";
			trigger_error("off-site tried to vote on non-topic upload, referer: ".$_SERVER['HTTP_REFERER'], E_USER_WARNING);
			return;
	    }
	    
	    // quietly snub them if they are not an admin and they are trying to post to the changeblog
	    if(array_key_exists("submit", $_REQUEST) && $upload->id() == "211604" && $_SESSION['status'] != "admin") {
	        return;
	    }
	    
		if( array_key_exists("submit", $_REQUEST)) {		    
			$fileid		 = array_key_exists("fileid", $_REQUEST) ?
							$_REQUEST['fileid'] : '';
			$comment	 = array_key_exists("comment", $_REQUEST) ?
							trim($_REQUEST['comment']) : "";
			$vote		 = array_key_exists("vote", $_REQUEST) ?
							$_REQUEST['vote'] : null;
			$offensive	 = array_key_exists("offensive", $_REQUEST) && $_REQUEST['offensive'] != "" ?
							1 : 0;
			$repost		 = array_key_exists("repost", $_REQUEST) 	&& $_REQUEST['repost'] != "" ?
							1 : 0;
			$subscribe	 = array_key_exists("subscribe", $_REQUEST) && $_REQUEST['subscribe'] != "" ?
							1 : 0;
			
			postComment($fileid, $vote, $repost, $offensive, $comment, $subscribe);

			if( array_key_exists("redirect", $_REQUEST) ) {
				header( "Location: /offensive/pages/pic.php?id=" . $_REQUEST['fileid'] );
				exit;
			}
			
			// redirecting to the same place prevents 'reload' from reposting the comment.
			header( "Location: " . $_SERVER['PHP_SELF'] . "?c=comments&fileid=" . $_REQUEST['fileid'] . (isset($commentid) ? "#".$commentid : "") );
			exit;
		}
	}

	function head() {
?>
	<script type="text/javascript" src="/offensive/js/jquery-1.2.6.min.js"></script>

	<script type="text/javascript" src="/offensive/js/subscriptions.js"></script>

	<script type="text/javascript">
		$(document).ready(function() {
			$("#subscribeLink").click(function(e) {
				handle_subscribe($(this),e,$("#fileid").attr("value"));
			});
			$("#unsubscribeLink").click(function(e) {
				handle_subscribe($(this),e,$("#fileid").attr("value"));
			});
		});
	</script>
		
<?
	}
	
	function toggleThumbnailLink() {
		global $me;

		?><div style="margin:4px;font-weight:bold"><?
			if($me->getPref("thumbnails_in_comments") == 1 || $me->getPref("thumbnails_in_comments") === false) {
				?><a href="setPref.php?p=thumbnails_in_comments&v=0">hide thumbnail</a><?
			}
			else {
				?><a href="setPref.php?p=thumbnails_in_comments&v=1">show thumbnail</a><?
			}
		?></div><?

	}

	function body() {
		global $upload, $uploader, $me;
	
		$subscribed = subscribed($upload->id());

		$comments_exist = count($upload->getComments()) > 0;
	
		$comments_heading = "the dorks who came before you said:";
		$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:";

		$href = "/offensive/pages/pic.php?id=".$upload->id();
?>	


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

		?><br/><span style="color:#666699"><?= $prefix ?> <a href="./?c=user&userid=<?= $uploader->id() ?>" style="color:#666699"><?= $uploader->username() ?></a> started it.</span><br/>
<?
	} else if( $upload->type() == 'audio' ) {
		?><a class="heading" id="pic" href="images/audio/<?= $upload->filename() ?>"><?= $upload->htmlFilename() ?></a><br/><span style="color:#666699">uploaded by <a href="./?c=user&userid=<?= $uploader->id() ?>" style="color:#666699"><?= $uploader->username() ?></a></span><br/><?
	} else {
?>
		<a class="heading" id="pic" href="<?= $href ?>"><?= $upload->htmlFilename() ?></a><br/><span style="color:#666699">uploaded by <a href="./?c=user&userid=<?= $uploader->id() ?>" style="color:#666699"><?= $uploader->username() ?></a></span><br/>
<?
	}	
?></div>
	
<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div class="bluebox" style="text-align:left">	

<?php 

	$commentnum = 0;
	if( $comments_exist > 0) { 
        // changeblog doesn't say anything demeaning about the people who posted.
        if($upload->id() != "211604") { 
			echo "<b>$comments_heading</b>";
        }

		global $warudo;
		$numgood = 0;
		$numbad = 0;
		foreach($upload->getComments() as $comment) {
			$commenter = $comment->commenter();
		?>
		<a name="<?= $comment->id() ?>"></a>
		<div class="entry u<?= $commenter->id() ?>" style="<?= nextStyle($commentnum++)?>">
    	
				<?php
    	
					// XXX: this output stuff all gets to change.
					if(!$me->squelched($commenter)) {
						echo $comment->HTMLcomment();
					}
					else {
						echo "<div class='squelched'>[ squelched ]</div>";
					}
					?>
    	            <br/>

				<div class="timestamp"><a href="#<?= $comment->id() ?>"><?= $comment->timestamp() ?></a></div>
				
				&raquo; <a href="./?c=user&userid=<?= $commenter->id() ?>"><?= $commenter->username() ?></a>
				
				<?
							
				if( $comment->vote() ) {
					$thevote = $comment->vote();
    	
					$numgood += ($thevote == 'this is good') ? 1 : 0;
					$numbad += ($thevote == 'this is bad') ? 1 : 0;
    	
					echo "<span title=\"+$numgood -$numbad\" class='vote" . additionalCssFor( $thevote ) . "'> [ " . $thevote . " ]</span>";
				}
				
				
				if( $comment->tmbo() == 1 ) {
					?><span class="vote"> [ this might be offensive ]</span><?php
				}
				
				if( $comment->tiar() == 1 ) {
					?><span class="vote"> [ this is a repost ]</span><?php
				}
    	
				?>
		</div>
		
		<?php 
		// end of comments loop
		} 
	
	// end of "are there any comments"
	}
	
		if( $upload->type() == 'image' || $upload->type() == 'avatar' ) {
			?>
				<div style="<?= nextStyle($commentnum++) ?>; border-top:1px solid #000033">
					<div style="float:right"><? toggleThumbnailLink() ?></div>
					<?
						if($me->getPref("thumbnails_in_comments") == 1 || $me->getPref("thumbnails_in_comments") === false) {
							if($me->squelched($uploader) || 
							    ($upload->is_nsfw() == 1 && $me->getPref("hide_nsfw") == 1) || 
							    ($upload->is_tmbo() == 1 && $me->getPref("hide_tmbo") == 1) ) {
								?><div style="padding:40px;">[ thumbnail filtered ]</div><?
							}
							else {
								?><a href="<?= $href ?>"><img style="margin:12px;" src="<?= $upload->thumbURL() ?>"/></a><?
							}
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
					<? if( $subscribed ) { ?>
						<b><a id="unsubscribeLink" href="subscribe.php?un=1&fileid=<?= $upload->id() ?>" title="take this thread off my 'unread comments' watch list." class="orange">unsubscribe</a></b>
					<? }
						else { ?>
						<b><a id="subscribeLink" href="subscribe.php?fileid=<?= $upload->id() ?>" title="watch this thread for new comments." class="orange">subscribe</a></b>
					<?	}
					?>
				</div>

					<div>
					<? if ( $upload->type() == "image" || $upload->type() == "avatar" ) { ?>
						<a class="heading" id="pic" href="pages/pic.php?id=<?= $upload->id() ?>"><?= $upload->htmlFilename(); ?></a><br/>
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
    if($upload->id() != "211604" || $_SESSION['status'] == "admin") {
?>

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading"><?= $add_comment_heading?></div>
			<div class="bluebox" style="text-align:center">			
			<a name="form"></a>
			<form method="POST" action="<?php echo $_SERVER['PHP_SELF']?>">

			
				<p>
					<input type="hidden" id="fileid" name="fileid" value="<?= $upload->id() ?>"/>
					<input type="hidden" name="c" value="comments"/>
					<textarea name="comment" rows="12" cols="64"></textarea>
				</p>
				


<?php 

	if( canVote($upload->id()) && $upload->file() && $upload->type() != 'topic' ) {
	
	?>	<div style="text-align:left;margin-left:10%">
			<input type="radio" name="vote" value="" checked="checked" id="novote"/>
			<br/>			
	<?php

		// show vote options	
		// oh jesus why are we doing this /o\
		$sql = "SHOW COLUMNS FROM offensive_comments LIKE 'vote'";
		$result = tmbo_query( $sql );
		$row = mysql_fetch_row($result);
		$options = explode("','",preg_replace("/(enum|set)\('(.+?)'\)/","\\2",$row[1]));
		
		
		foreach( $options as $option ) {
			?>
				<input type="radio" name="vote" value="<?php echo $option ?>" id="<?php echo $option?>"/>
				<label for="<?php echo $option?>">[ <?php echo $option?> ]</label><br/>
			
			<?php
		}
	?>
	
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

<?
    }
}

	function nextStyle($i) {
		global $style;
	    $style = $i % 2 ? "background:#bbbbee;" : "background:#ccccff";
		return $style;
	}


?>
