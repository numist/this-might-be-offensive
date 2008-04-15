<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/getPrefs.inc' );
	require_once( 'offensive/assets/tabs.inc' );

	function addBookmarks( $fileid, $commentid ) {
		global $link;
		if(!isset($link) || !$link) $link = openDbConnection();
		
		$sql = "UPDATE offensive_subscriptions SET commentid = $commentid WHERE fileid = $fileid && commentid IS NULL";
					
		$result = tmbo_query( $sql );

	}

	function additionalCssFor( $vote ) {
		
		if( strpos( $vote, 'bad' ) > 0 ) {
			return " bad";
		}

		if( strpos( $vote, 'good' ) > 0 ) {
			return " good";
		}
		
		return "";
	}


	
	function subscribe( $fileid, $uid ) {
		global $link;
		if( ! is_numeric( $uid ) || ! is_numeric( $fileid ) ) {
			return;
		}
		if(!isset($link) || !$link) $link = openDbConnection();

		$sql = "SELECT * FROM offensive_subscriptions WHERE userid = $uid AND fileid = $fileid";
		if(mysql_num_rows(tmbo_query($sql)) > 0) return;

		$sql = "insert into offensive_subscriptions (userid, fileid) VALUES ($uid, $fileid)";
		$result = tmbo_query( $sql );
	}

	function alreadyVoted( $uid, $fid ) {
		$sql = "SELECT count( vote ) AS thecount FROM offensive_comments WHERE fileid=$fid AND userid=$uid AND vote LIKE 'this%'";
		$result = tmbo_query( $sql );
		$row = mysql_fetch_assoc( $result );
		
		$voted = ( $row[ 'thecount' ] > 0 );
		return $voted;
	}

	function clearBookmarks( $userid, $fileid ) {
		global $link;
		if( (! is_numeric( $userid ) ) || (! is_numeric( $fileid ) ) ) {
			return;
		}
		if(!isset($link) || !$link) $link = openDbConnection();
		$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$userid and fileid=$fileid";
		$result = tmbo_query( $sql );

	}
	
	function title() {
		global $filename;
		return "[ tmbo ] : " . $filename;
	}
	
	function updateCommentCount( $threadid, $good, $bad, $repost, $tmbo, $comment ) {
		global $link;

		$usrid = $_SESSION['userid'];
		$sql = "INSERT INTO offensive_count_cache ( threadid, good, bad, repost, tmbo, comments )
					VALUES ( $threadid, $good, $bad, $repost, $tmbo, $comment )
					ON DUPLICATE KEY UPDATE
							good = good + $good,
							bad = bad + $bad,
							repost = repost + $repost,
							tmbo = tmbo + $tmbo,
							comments = comments + $comment";
		
		if(!isset($link) || !$link) $link = openDbConnection();
		$result = tmbo_query( $sql );

	}

	function start() {
		global $filename, $link;
	
		mustLogIn();

		if( ! is_numeric( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$usrid = $_SESSION['userid'];
		$fileid = $_REQUEST['fileid'];
		
		clearBookmarks( $usrid, $fileid );

		if(!isset($link) || !$link) $link = openDbConnection();

		$sql = "SELECT filename, nsfw, tmbo from offensive_uploads where id=$fileid";
		$result = tmbo_query( $sql );
		list( $filename, $nsfw, $tmbo ) = mysql_fetch_array( $result );
	
	    // check to see if someone's posting.  
	    
	    /* check to see if someone is being hoodwinked by a link in comments
	     * or outside the site's borders. while we're at it, let them know.
	     * NB: only works if the client is reporting REFERERS. 
	     */
	    if(array_key_exists("submit", $_GET) && array_key_exists("HTTP_REFERER", $_SERVER) && 
	       trim($_SERVER['HTTP_REFERER']) !== "" &&
	       (strpos($_SERVER['HTTP_REFERER'], $_SERVER['SERVER_NAME']."/offensive/?c=comments&fileid=") !== false ||
	       strpos($_SERVER['HTTP_REFERER'], $_SERVER['SERVER_NAME']."/offensive/index.php?c=comments&fileid=") !== false ||
	       strpos($_SERVER['HTTP_REFERER'], "//".$_SERVER['SERVER_NAME']."/") === false)) {
	        echo "you've been had.";
	        trigger_error("link in discussions tried to vote", E_USER_WARNING);
	        return;
	    }
	    
	    // also quietly snub them if they are not an admin and they are trying to post to the changeblog
	    if(array_key_exists("submit", $_REQUEST) && $fileid == "211604" && $_SESSION['status'] != "admin") {
	        return;
	    }
	    
		if( array_key_exists("submit", $_REQUEST)) {		    
			$comment = isset($_REQUEST['comment']) ? trim($_REQUEST['comment']) : "";
			
			$already_voted = alreadyVoted( $_SESSION['userid'], $_REQUEST['fileid'] );
						
			$sql = "SELECT userid FROM offensive_uploads WHERE id=" . $_REQUEST['fileid'];
			$res = tmbo_query( $sql );
			$row = mysql_fetch_assoc( $res );
			if( $already_voted || $row['userid'] == $_SESSION['userid'] || (isset($expired) && $expired) ) {
				$vote = null;
			}
			else {
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
				$commentid = mysql_insert_id( $link );
				subscribe( $fileid, $_SESSION['userid'] );
				if( strlen( $comment ) > 0 ) {
					addBookmarks( $fileid, $commentid );
				}
			}
			
			updateCommentCount( $fileid, ($vote === 'this is good') ? 1 : 0,
								($vote == 'this is bad') ? 1 : 0,
								$repost,
								$offensive,
								strlen( $comment ) > 0 ? 1 : 0 );

			if( array_key_exists("redirect", $_REQUEST) ) {
				if( isset($_SERVER['HTTP_REFERER']) && 
				    strpos( $_SERVER['HTTP_REFERER'], 'audio' ) !== false ) {
					header( "Location: " . $_SERVER['HTTP_REFERER'] );
					exit;
				}
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
	<script type="text/javascript">
		
		function handleKeyDown( e ) {

			if( e == null || e.which == null ) {
				return true;
			}
			
			var id;
			
			switch( e.which ) {
			
				case 38:
					id = "pic";
				break;

			}
			
			if( id ) {
				document.location.href = document.getElementById( id ).href;
			}
			return false;
		
		}
		
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
		global $link;
	
	if(!isset($link) || !$link) $link = openDbConnection();

	$fid = $_REQUEST['fileid'];
	$sql = "SELECT * FROM offensive_subscriptions
		WHERE userid=" . $_SESSION['userid'] . " 
		&& fileid=$fid";
	$subscribed = mysql_num_rows( tmbo_query($sql) ) > 0 ? true : false;

	$sql = "SELECT users.username, users.userid, users.account_status, offensive_uploads.filename, nsfw, tmbo, offensive_uploads.timestamp as upload_timestamp, offensive_uploads.type
				FROM users, offensive_uploads
				WHERE users.userid = offensive_uploads.userid
				AND offensive_uploads.id = " . $_REQUEST['fileid'];
	$result = tmbo_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$uploader = $row['username'];
	$filename = $row['filename'];
	$uploaderid = $row['userid'];
	$uploaderstatus = $row['account_status'];
	$uploadstamp = $row['upload_timestamp'];
	$nsfw = $row['nsfw'];
	$tmbo = $row['tmbo'];	
	$type = $row['type'];
	$expired = getFile($_REQUEST['fileid'], $filename, $uploadstamp, $type) == ''? true : false;

	$already_voted = $_SESSION['userid'] == $row['userid'];
	
	$sql = "SELECT offensive_uploads.filename AS filename, offensive_comments.*, offensive_comments.id as commentid, offensive_comments.timestamp AS comment_timestamp, users.*
				FROM offensive_uploads, offensive_comments, users
				WHERE users.userid = offensive_comments.userid
				AND offensive_uploads.id=fileid AND fileid = " . $_REQUEST['fileid'] . " 
				ORDER BY offensive_comments.timestamp";
	
	$result = tmbo_query( $sql );
	
	$comments_exist = mysql_num_rows( $result ) > 0;
	
	$comments_heading = "the dorks who came before you said:";
	$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:";

	$href = getFileURL($_REQUEST['fileid'], $filename, $uploadstamp, $type);

?>	


		<div class="heading">
<?
    // changeblog special header
	if($_REQUEST['fileid'] == "211604") {
		echo htmlEscape($filename); 
		?>
		<br/><span style="color:#666699">beat to fit. paint to match.</span><br/>
		<?
	} else if( $type == 'topic' ) {
		$prefix = $uploaderstatus == "admin" ? "" : "don't blame me,";

		echo htmlEscape($filename);

		?><br/><span style="color:#666699"><?= $prefix ?> <a href="./?c=user&userid=<?echo $uploaderid ?>" style="color:#666699"><?php echo $uploader?></a> started it.</span><br/>
<?
	} else if( $type == 'audio' ) {
		?><a class="heading" id="pic" href="images/audio/<?= $filename ?>"><?php 
		    echo htmlEscape($filename);
		?></a><br/><span style="color:#666699">uploaded by <a href="./?c=user&userid=<?echo $uploaderid ?>" style="color:#666699"><?php echo $uploader?></a></span><br/><?
	} else {
?>
		<a class="heading" id="pic" href="<?= $href ?>"><?php 
		    echo htmlEscape($filename);
		?></a><br/><span style="color:#666699">uploaded by <a href="./?c=user&userid=<?echo $uploaderid ?>" style="color:#666699"><?php echo $uploader?></a></span><br/>
<?
	}	
?>
			
		</div>
	
<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div class="bluebox" style="text-align:left">	

<?php if( $comments_exist ) { 
        // changeblog doesn't say anything demeaning about the people who posted.
        if($_REQUEST['fileid'] != "211604") { ?>		

		    <b><?php echo $comments_heading?></b>
<?php
        }

	global $warudo;
	$numgood = 0;
	$numbad = 0;
	$commentnum = 0;
	while( $row = mysql_fetch_assoc( $result ) ) {

		/*
		 * to disable warudo comment hack, comment out the next line of code.
		 * inspiration from: http://thismight.be/offensive/?c=comments&fileid=229329#6340701
		 */
	        $warudo = strpos($row['comment'], "ZA WARUDO") !== false;

	?>
	<a name="<?= $row['commentid'] ?>"></a>
	<div class="entry u<?= $row['userid'] ?>" style="<?php echo nextStyle($commentnum++)?>">

			<?php
			
				$comment = htmlEscape($row['comment']);

/*				
				// darkstalker and zetsumei drama queen gag
				if( (! isset( $_REQUEST['dsgag'] ) )
					&& ($_SESSION['userid'] != 1613 && $_SESSION['userid'] != 3940)
					&& ($row['userid'] == 1613 || $row['userid'] == 3940 )
					&& strlen( $comment ) > 0 )  {
					$comment = "Hey everybody! Look at me! Look at me! I'm leaving! See? Here I go! Don't try to stop me!";
				}
*/

				if( ! isSquelched( $row['userid'] ) ) {
				    
				    // remisser entrapment url gag
				    if($_SESSION['userid'] != 2935 && $row['userid'] == 2935) {
				        $pattern = "/(http[s]*:\/\/[^\s<>]+)/i";
                    	$replacement = "http://preview.tinyurl.com/2tx";
                    	$comment = preg_replace( $pattern, $replacement, $comment );
				    }
				    
					echo nl2br( linkUrls( $comment ) );
				}
				else {
					echo "<div class='squelched'>[ squelched ]</div>";
				}
				?>
                <br/>
					 
 	 			<div class="timestamp"><a href="#<?= $row['commentid'] ?>"><?= $row['comment_timestamp'] ?></a></div>
				&raquo; 

			<?php

			echo "<a href=\"./?c=user&userid=" . $row['userid'] . "\">" . $row['username'] . "</a>";
						
			if( $row['vote'] ) {
				$thevote = $row['vote'];

				$numgood += ($thevote == 'this is good') ? 1 : 0;
				$numbad += ($thevote == 'this is bad') ? 1 : 0;

				echo "<span title=\"+$numgood -$numbad\" class='vote" . additionalCssFor( $thevote ) . "'> [ " . $thevote . " ]</span>";
				
				if( $row['userid'] == $_SESSION['userid'] ) {
					$already_voted = true;
				}		
			}
			
			
			if( $row['offensive'] == 1 ) {
				?><span class="vote"> [ this might be offensive ]</span><?php
			}
			
			if( $row['repost'] == 1 ) {
				?><span class="vote"> [ this is a repost ]</span><?php
			}
/*
			if( ! isSquelched( $row['userid'] ) ) {
				?><a href="setPref.php?sq=<?= $row['userid'] ?>" class="squelched" style="margin-left:8px">squelch</a><?
			}
			else {
				?><a href="setPref.php?unsq=<?= $row['userid'] ?>" class="squelched" style="margin-left:8px">unsquelch</a><?
			}
*/
			?>
	</div>
	
	<?php } 
	
	}
	
		if( $type == 'image' || $type == 'avatar' ) {
			?>
				<div style="<?= nextStyle($commentnum++) ?>; border-top:1px solid #000033">
					<div style="float:right"><? toggleThumbnailLink() ?></div>
					<?
						if( showThumbnails() ) {
							if( ( isSquelched( $uploaderid ) ) || 
							    array_key_exists("prefs", $_SESSION) &&
							    is_array($_SESSION['prefs']) &&
							    ($nsfw == 1 && 
							    array_key_exists("hide nsfw", $_SESSION['prefs']) &&
							    $_SESSION['prefs']['hide nsfw'] == 1) || 
							    ($tmbo == 1 && 
							    array_key_exists("hide tmbo", $_SESSION['prefs']) &&
							    $_SESSION['prefs']['hide tmbo'] == 1) ) {
								?><div style="padding:40px;">[ thumbnail filtered ]</div><?
							}
							else {
								?><a href="<?= $href ?>"><img style="margin:12px;" src="<?= getThumbURL($_REQUEST['fileid'], $filename, $uploadstamp, $type); ?>"/></a><?
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
						<b><a href="subscribe.php?un=1&fileid=<?= $fid ?>" title="take this thread off my 'unread comments' watch list." class="orange">unsubscribe</a></b>
					<? }
						else { ?>
						<b><a href="subscribe.php?fileid=<?= $fid ?>" title="watch this thread for new comments." class="orange">subscribe</a></b>
					<?	}
					?>
				</div>

					<div>
					<? if ( $type == 'image' ) { ?>
						<a class="heading" id="pic" href="pages/pic.php?id=<?php echo $_REQUEST['fileid']?>"><?php 
						echo htmlEscape($filename);
						?></a><br/>
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
    if($_REQUEST['fileid'] != "211604" || $_SESSION['status'] == "admin") {
?>

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading"><?php echo $add_comment_heading?></div>
			<div class="bluebox" style="text-align:center">			
			<a name="form"></a>
			<form method="POST" action="<?php echo $_SERVER['PHP_SELF']?>">

			
				<p>
					<input type="hidden" name="fileid" value="<?php echo $_REQUEST['fileid']?>"/>
					<input type="hidden" name="c" value="comments"/>
					<textarea name="comment" style="width:80%;height:150px;"></textarea>
				</p>
				


<?php 

	if( !$already_voted && !$expired && $type != 'topic' ) {
	
	?>	<div style="text-align:left;margin-left:10%">
			<input type="radio" name="vote" value="" checked="checked" id="novote"/>
			<br/>			
	<?php

		// show vote options	
		
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
		global $style, $warudo;
		if($warudo) {
		    $style = $i % 2 ? "background:#444411;" : "background:#333300";
		} else {
		    $style = $i % 2 ? "background:#bbbbee;" : "background:#ccccff";
		}
		return $style;
	}


?>