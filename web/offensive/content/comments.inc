<?php
	// Include, and check we've got a connection to the database.
	require_once("admin/mysqlConnectionInfo.inc");
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/getPrefs.inc");
	require_once("offensive/assets/tabs.inc");
	require_once("offensive/assets/comments.inc");
	require_once("offensive/assets/classes.inc");

// XXX: [tpg] — allow current winner to vote current image as new winner if [tpg] tag exists

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
		global $upload, $uploader, $expired;
	
		mustLogIn();

		if( ! is_numeric( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$usrid = $_SESSION['userid'];
		$fileid = $_REQUEST['fileid'];
		
				
		clearSubscription($fileid, $usrid);
		
		$upload = new Upload($fileid);
		$uploader = $upload->uploader();

		if(!$upload->exists()) {
			require("offensive/404.php");
			exit;
		}
		
		$expired = $upload->file() == ''? true : false;
	
	    // check to see if someone's posting.  
	    /* check if someone is being hoodwinked by a link in comments
	     * or outside the site's borders. while we're at it, let them know.
	     * NB: only works if the client is reporting REFERERS. 
		 * NNBB: this code will allow cheater-voting on discussions, since 
		 * that's an easter egg anyway.
	     */
	    if($upload->type() != 'topic' &&
		   array_key_exists("submit", $_GET) && array_key_exists("HTTP_REFERER", $_SERVER) && 
	       trim($_SERVER['HTTP_REFERER']) !== "" &&
	       (strpos($_SERVER['HTTP_REFERER'], $_SERVER['SERVER_NAME']."/offensive/?c=comments&fileid=") !== false ||
	       strpos($_SERVER['HTTP_REFERER'], $_SERVER['SERVER_NAME']."/offensive/index.php?c=comments&fileid=") !== false ||
	       strpos($_SERVER['HTTP_REFERER'], "//".$_SERVER['SERVER_NAME']."/") === false)) {
	        echo "you've been had.";
	        trigger_error("link in discussions tried to vote", E_USER_WARNING);
	        return;
	    }
	    
	    // quietly snub them if they are not an admin and they are trying to post to the changeblog
	    if(array_key_exists("submit", $_REQUEST) && $upload->id() == "211604" && $_SESSION['status'] != "admin") {
	        return;
	    }
	    
		if( array_key_exists("submit", $_REQUEST)) {		    
			$comment = isset($_REQUEST['comment']) ? trim($_REQUEST['comment']) : "";
			
			// prevent doubleposts, but only content doubleposts within 30 seconds
			$sql = "SELECT id, fileid, timestamp, comment, NOW() as curtime FROM offensive_comments WHERE userid = ".$_SESSION['userid'].
			       " ORDER BY timestamp DESC LIMIT 1";
			$res = tmbo_query($sql);
			$row = mysql_fetch_assoc($res);
			if($row['fileid'] == $fileid &&
			   $row['comment'] == $comment && 
			   $comment != "" &&
			   strtotime($row['curtime']) - strtotime($row['timestamp']) < 30) {
				header("Location: ./?c=comments&fileid=".$fileid."#".$row['id']);
				exit;
			}
			
			$already_voted = alreadyVoted( $_REQUEST['fileid'], $_SESSION['userid'] );
			
			if( $already_voted || $uploader->id() == $_SESSION['userid'] || $upload->file() == "" ) {
				$vote = null;
			} else {
				$vote = array_key_exists("vote", $_REQUEST) ? $_REQUEST['vote'] : null;
			}

			$offensive = array_key_exists("offensive", $_REQUEST) && $_REQUEST['offensive'] != "" ? 1 : 0;
			$repost = array_key_exists("repost", $_REQUEST) && $_REQUEST['repost'] != "" ? 1 : 0;
			$subscribe = array_key_exists("subscribe", $_REQUEST) && $_REQUEST['subscribe'] != "" ? 1 : 0;
			$sql = "INSERT INTO offensive_comments ( userid, fileid, comment, vote, offensive, repost, user_ip )";
			$sql .= " VALUES ( $usrid, $fileid, '".sqlEscape($comment)."', '$vote', $offensive, $repost, '" . sqlEscape($_SERVER['REMOTE_ADDR']) . "')";
		
			if( ! ($comment == "" && $vote == null && $offensive == 0 && $repost == 0) ) {
				$result = tmbo_query( $sql );
			}

			if( strlen( $comment ) > 0 || $vote == 'this is bad' || $subscribe ) {
				$commentid = mysql_insert_id();
				subscribe( $fileid, $_SESSION['userid'] );
				if( strlen( $comment ) > 0 ) {
					updateSubscriptions( $fileid, $commentid );
				}
			}
			
			updateCommentCount( $fileid, ($vote === 'this is good') ? 1 : 0,
								($vote == 'this is bad') ? 1 : 0,
								$repost,
								$offensive,
								strlen( $comment ) > 0 ? 1 : 0 );

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
	
	function showThumbnails() {
		return ((! isset( $_SESSION['prefs']['thumbnails_in_comments'] )) || $_SESSION['prefs']['thumbnails_in_comments'] == 1);
	}
	
	function toggleThumbnailLink() {

		?><div style="margin:4px;font-weight:bold"><?
			if( showThumbnails() ) {
				?><a href="setPref.php?p=9&v=10">hide thumbnail</a><?
			}
			else {
				?><a href="setPref.php?p=9&v=">show thumbnail</a><?
			}
		?></div><?

	}

	function body() {
		global $expired, $upload, $uploader;
	
		$subscribed = subscribed($upload->id());

		$comments_exist = count($upload->getComments()) > 0;
	
		$comments_heading = "the dorks who came before you said:";
		$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:";

		$href = "/offensive/pages/pic.php?id=".$upload->id();
?>	


		<div class="heading"><?
    // changeblog special header
	if($upload->id() == "211604") {
		echo htmlFilename($row); 
		?>
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
				
					$commenttext = htmlEscape($comment->comment());
    	
					if( ! isSquelched( $commenter->id() ) ) {
						echo nl2br( linkUrls( $commenttext ) );
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
					
					if( $commenter->id() == $_SESSION['userid'] ) {
						$already_voted = true;
					}
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
	
	$already_voted = $uploader->id() == $_SESSION["userid"] ? true : $already_voted;
	
		if( $upload->type() == 'image' || $upload->type() == 'avatar' ) {
			?>
				<div style="<?= nextStyle($commentnum++) ?>; border-top:1px solid #000033">
					<div style="float:right"><? toggleThumbnailLink() ?></div>
					<?
						if( showThumbnails() ) {
							if( ( isSquelched( $uploader->id() ) ) || 
							    array_key_exists("prefs", $_SESSION) &&
							    is_array($_SESSION['prefs']) &&
							    ($upload->is_nsfw() == 1 && 
							    array_key_exists("hide nsfw", $_SESSION['prefs']) &&
							    $_SESSION['prefs']['hide nsfw'] == 1) || 
							    ($upload->is_tmbo() == 1 && 
							    array_key_exists("hide tmbo", $_SESSION['prefs']) &&
							    $_SESSION['prefs']['hide tmbo'] == 1) ) {
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

	if( !$already_voted && $upload->file() && $type != 'topic' ) {
	
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