<?

require_once("offensive/assets/classes.inc");

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

function postComment($fileid, $vote, $repost, $offensive, $comment, $subscribe=null) {
	$userid = me() ? me()->id() : null;
	if($userid == null) return false;

	if($vote != "this is good" && $vote != "this is bad" && $vote != "") {
		trigger_error("invalid vote $vote", E_USER_ERROR);
	}
	
	$comment = trim($comment);

	$upload = new Upload($fileid);
	if(!$upload->exists()) trigger_error("invalid fileid ($fileid)", E_USER_ERROR);

	// prevent double-votes, self-voting, and disallowed things
	if(!$upload->canVote()) {
		$vote = "";
		$repost = 0;
		$offensive = 0;
	}
	if(!$upload->canComment()) {
		$comment = "";
	}
	
	// sometimes, subscribe the commenter.
	if($comment || $vote == "this is bad" || $subscribe) {
		$upload->subscribe();
	}

	if(!$upload->canComment()) {
		return false;
	}

	// prevent empty comments from being inserted
	if(!($comment || $vote || $offensive || $repost)) {
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
		// update subscriptions
		$upload->updateSubscriptions($commentid);
		// the commenter doesn't need their subscription updated
		$upload->clearSubscription();
	}

	return $commentid;
}

?>
