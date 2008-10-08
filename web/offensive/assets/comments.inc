<?

/*
 * return if a user is capable of voting on a file
 * there are two reason a user would not be able to vote:
 *  1. the user uploaded the file
 *  2. the user has already voted
 */
function canVote($fileid, $userid=null) {
	if($userid == null) $userid = $_SESSION['userid'];
	
	if(alreadyVoted($fileid, $userid)) return false;
	
	$sql = "SELECT userid FROM offensive_uploads WHERE id = $fileid and userid = $userid";
	if(mysql_num_rows(tmbo_query($sql))) return false;
	
	return true;
}

/*
 * if a user has already voted on the file in the arguments, this function
 * will return true.
 */
function alreadyVoted($fileid, $userid=null) {
	if($userid == null) $userid = $_SESSION['userid'];
	
	$sql = "SELECT vote AS thecount FROM offensive_comments 
	        WHERE fileid=$fileid AND userid=$userid AND vote LIKE 'this%'";
	return (mysql_num_rows(tmbo_query($sql)) > 0);
}

/*
 * subscribes a user to a thread.  this done automatically in the code in a few cases:
 *  * the user commented on the file
 *  * the user voted "this is bad" on the file
 */
function subscribe($fileid, $userid=null) {
	if($userid == null) $userid = $_SESSION['userid'];
	
	// check if the user is already subscribed
	$sql = "SELECT * FROM offensive_subscriptions WHERE userid = $userid AND fileid = $fileid";
	if(mysql_num_rows(tmbo_query($sql)) > 0) return true;

	// ensure file exists
	$sql = "SELECT * FROM offensive_uploads WHERE id = $fileid";
	if(mysql_num_rows(tmbo_query($sql)) == 0) send(false);

    // finally, subscribe
	$sql = "INSERT INTO offensive_subscriptions (userid, fileid) VALUES ( $userid, $fileid )";
	tmbo_query( $sql );
	return true;
}

/*
 * unsubscribes a user from a thread.
 */
function unsubscribe($fileid, $userid=null) {
	if($userid == null) $userid = $_SESSION['userid'];
	
	$sql = "DELETE FROM offensive_subscriptions WHERE userid=$userid AND fileid=$fileid";
	tmbo_query($sql);

	return true;
}

function updateSubscriptions( $fileid, $commentid ) {
	$sql = "UPDATE offensive_subscriptions SET commentid = $commentid 
	        WHERE fileid = $fileid AND commentid IS NULL AND userid != ".$_SESSION['userid'];
	$result = tmbo_query( $sql );
}

function clearSubscription($fileid, $userid) {
	$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$userid and fileid=$fileid";
	$result = tmbo_query( $sql );
}

function updateCommentCount($fileid, $good, $bad, $repost, $tmbo, $comment) {
	$good = $good ? 1 : 0;
	$bad = $bad ? 1 : 0;
	$repost = $repost ? 1 : 0;
	$tmbo = $tmbo ? 1 : 0;
	$comment = $comment ? 1 : 0;

	$sql = "INSERT INTO offensive_count_cache ( threadid, good, bad, repost, tmbo, comments )
				VALUES ( $fileid, $good, $bad, $repost, $tmbo, $comment )
				ON DUPLICATE KEY UPDATE
						good = good + $good,
						bad = bad + $bad,
						repost = repost + $repost,
						tmbo = tmbo + $tmbo,
						comments = comments + $comment";
	
	tmbo_query( $sql );
}

function subscribed($fileid, $userid=null) {
	if($userid == null) $userid = $_SESSION['userid'];
	
	$sql = "SELECT * FROM offensive_subscriptions
		WHERE userid=$userid AND fileid=$fileid";
	return (mysql_num_rows(tmbo_query($sql)) > 0 ? true : false);
}

function getUnreadComments($userid) {
	// this will replace the functionality of api_unreadcomments() (api) and unread() (index)
	// maybe.
}

function postComment($fileid, $vote, $repost, $offensive, $comment) {
	$userid = $_SESSION['userid'];
	
	// XXX: arg checking for non-incoming args?
	if($vote != "this is good" && $vote != "this is bad" && $vote != "") {
		trigger_error("invalid vote $vote", E_USER_ERROR);
	}
	
	// check to see if the fileid is valid
	$sql = "SELECT filename, nsfw, tmbo, timestamp, type FROM offensive_uploads WHERE id=$fileid";
	if(mysql_num_rows(tmbo_query( $sql )) == 0) trigger_error("invalid fileid ($fileid)", E_USER_ERROR);
	
	// prevent double-votes and self-voting
	if(!canVote($fileid)) {
		$vote = "";
		$repost = 0;
		$tmbo = 0;
	}
	
	// prevent empty comments from being inserted
	if(!($comment || $vote || $offensive || $repost)) {
		return;
	}
	
	// protect the changeblog
	if($fileid == "211604" && $_SESSION['status'] != "admin") return;

	// sanitize the comment
	if($comment) $comment = trim(sqlEscape($comment));
	
	// update the offensive_count_cache
	$good_count = ($vote == "this is good") ? 1 : 0;
	$bad_count = ($vote == "this is bad") ? 1 : 0;
	$comment_count = (strlen( $comment ) > 0 ? 1 : 0 );
	if(!$repost) $repost = 0;
	if(!$offensive) $offensive = 0;
	updateCommentCount($fileid, $good_count, $bad_count, $repost, $offensive, $comment_count);

	// add the comment to the database
	$sql = "INSERT INTO offensive_comments ( userid, fileid, comment, vote, offensive, repost, user_ip ) 
	        VALUES ( $userid, $fileid, '$comment', '$vote', $offensive, $repost, '".sqlEscape($_SERVER['REMOTE_ADDR'])."')";
	tmbo_query($sql);
	$commentid = mysql_insert_id();
	
	if($comment != false || $vote == "this is bad") {
		subscribe($fileid);
		if($comment != false && $commentid != 0) {
			updateSubscriptions($fileid, $commentid);
		} else if ($commentid == 0) {
			trigger_error("commentid was 0?", E_USER_ERROR);
		}
	}
}

function searchComments($q, $limit) {
	// pending search fixes
}

?>
