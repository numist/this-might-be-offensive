<?

require_once("offensive/assets/classes.inc");

/*
 * unsubscribes a user from a thread.
 */
function unsubscribe($fileid) {
	$userid = me() ? me()->id() : null;
	if($userid == null) return false;
		
	$sql = "DELETE FROM offensive_subscriptions WHERE userid=$userid AND fileid=$fileid";
	tmbo_query($sql);

	return true;
}

function updateSubscriptions( $fileid, $commentid ) {
	$sql = "UPDATE offensive_subscriptions SET commentid = $commentid 
	        WHERE fileid = $fileid AND commentid IS NULL";
	$result = tmbo_query( $sql );
}

function clearSubscription($fileid, $userid) {
	$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid=$userid and fileid=$fileid";
	$result = tmbo_query( $sql );
	
	if(mysql_affected_rows() < 1) { 
		return false;
	}
	return true;
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
	insertChange("comment", $fileid, "{$good}:{$bad}:{$repost}:{$tmbo}:$comment");
	tmbo_query( $sql );
}

function getUnreadComments($userid) {
	// this will replace the functionality of api_unreadcomments() (api) and unread() (index)
	// maybe.
}

function postComment($fileid, $vote, $repost, $offensive, $comment, $subscribe=null) {
	$userid = me() ? me()->id() : null;
	if($userid == null) return false;

	if($vote != "this is good" && $vote != "this is bad" && $vote != "") {
		trigger_error("invalid vote $vote", E_USER_ERROR);
	}
	
	$comment = trim($comment);

	// check to see if the fileid is valid
	// XXX: this can be done better with an Upload object
	$sql = "SELECT filename, nsfw, tmbo, timestamp, type FROM offensive_uploads WHERE id=$fileid";
	if(mysql_num_rows(tmbo_query( $sql )) == 0) trigger_error("invalid fileid ($fileid)", E_USER_ERROR);

	// prevent double-votes and self-voting
	if(!id(new Upload($fileid))->canVote()) {
		$vote = "";
		$repost = 0;
		$offensive = 0;
	}
	
	if(!id(new Upload($fileid))->canComment()) {
		$comment = "";
	}

	// sometimes, subscribe the commenter.
	if($comment || $vote == "this is bad" || $subscribe) {
		id(new Upload($fileid))->subscribe();
	}

	// prevent empty comments from being inserted
	if(!($comment || $vote || $offensive || $repost)) {
		return false;
	}

	// changeblog
	if($fileid == "211604" && me()->status() != "admin") {
		trigger_error("user ".me()->username()." not allowed to post to changeblog", E_USER_NOTICE);
		return false;
	}

	// rhi-rhi needs ipod
	if($fileid == "197431") {
		return false;
	}

	// prevent doubleposts, but only content doubleposts within 30 seconds
	$sql = "SELECT id, fileid, timestamp, comment, NOW() as curtime 
	        FROM offensive_comments 
	        WHERE userid = $userid ORDER BY timestamp DESC LIMIT 1";
	$res = tmbo_query($sql);
	$row = mysql_fetch_assoc($res);
	if($row['fileid'] == $fileid &&
	   $row['comment'] == $comment && 
	   $comment != "" &&
	   strtotime($row['curtime']) - strtotime($row['timestamp']) < 60) {
		trigger_error("caught doublepost", E_USER_NOTICE);
		return false;
	}

	// sanitize the comment
	if($comment) {
		// escape it for sql if the comment is not empty
		$comment = sqlEscape($comment);
	}

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

	if($comment) {
		// update all subscriptions
		updateSubscriptions($fileid, $commentid);
		// mark your own as read
		clearSubscription($fileid, me()->id());
	}

	return $commentid;
}

?>
