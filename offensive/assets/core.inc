<?
	// XXX: check that we actually need all these
	require_once( 'offensive/assets/header.inc' );
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/argvalidation.inc");
	
	function uploadsql() {
		$subscribed = me()
		              ? "("."SELECT COUNT(*) FROM offensive_subscriptions 
		                     WHERE fileid = up.id AND userid = ".me()->id().")"
		              : "NULL";
		
		return "SELECT up.*,
		               $subscribed as subscribed,
			             ca.good as goods,
			             ca.bad as bads,
			             ca.tmbo as tmbos,
			             ca.repost as tiars,
			             ca.comments,
			             ca.timestamp as last_active,
			             u.username as user_username,
			             u.account_status as user_account_status
		        FROM offensive_uploads up, offensive_count_cache ca, users u
		        WHERE ca.threadid = up.id AND up.userid = u.userid AND up.status = 'normal'";
	}
	
	function commentsql() {
	 	return "SELECT com.*,
	                 u.username as user_username, u.account_status as user_account_status, u.userid as user_userid,
	                 up.userid as upload_userid, up.id as upload_id, up.filename as upload_filename,
	                 up.timestamp as upload_timestamp, up.nsfw as upload_nsfw, up.tmbo as upload_tmbo,
	                 up.type as upload_type, up.status as upload_status
	          FROM offensive_comments com, users u, offensive_uploads up
	          WHERE com.userid = u.userid AND up.id = com.fileid";
	}
	
/**
	common functions
**/
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

/**
	core functions
**/	
	global $types;
	$types = array("image", "topic", "avatar", "audio");

	function core_getuploads($args) {
		global $types;
		
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}

		$type =   check_arg("type",   "string",  $args, false, $types);
		$userid = check_arg("userid", "integer", $args, false);
		$after =  check_arg("after",  "date",    $args, false);
		$before = check_arg("before", "date",    $args, false);
		$max =    check_arg("max",    "integer", $args, false);
		$since =  check_arg("since",  "integer", $args, false);
		$sort =   check_arg("sort",   "string",  $args, false, array("date_desc", "date_asc", "votes_asc", "votes_desc", "comments_asc", "comments_desc", "activity_asc", "activity_desc"));
		$limit =  check_arg("limit",  "limit",   $args, false);
		$p =      check_arg("p",      "integer", $args, false);
		$nsfw =   check_arg("nsfw",   "integer", $args, false, array(0,1));
		$tmbo =   check_arg("tmbo",   "integer", $args, false, array(0,1));
		handle_errors();
		
		// sort order needs to always be set, even if only to default.
		if($sort === false) $sort = "date_desc";
		
		$sql = uploadsql();
		
		if($type !== false) {
			$sql .= " AND up.type = '$type'";
		}
		if($userid !== false) {
			$sql .= " AND up.userid = $userid";
		}
		if($after !== false) {
			$sql .= " AND up.timestamp > '$after'";
		}
		if($before !== false) {
			$sql .= " AND up.timestamp < '$before'";
		}
		if($max !== false) {
			$sql .= " AND up.id <= $max";
		}
		if($since !== false) {
			$sql .= " AND up.id > $since";
		}
		if($nsfw !== false) {
			if($nsfw) {
				$sql .= " AND up.nsfw = 1";
			} else {
				$sql .= " AND up.nsfw = 0";
			}
		}
		if($tmbo !== false) {
			if($tmbo) {
				$sql .= " AND up.tmbo = 1";
			} else {
				$sql .= " AND up.tmbo = 0";
			}
		}
		switch($sort) {
			// XXX: date orders have two order clauses because...  well because otherwise the query takes half a minute, even with the correct indices explicitly in the query D:
			case "date_desc":
				$sql .= " ORDER BY up.timestamp DESC, 
						  up.id DESC";
				// make the common case fast
				$sql = str_replace("offensive_uploads up", "offensive_uploads up USE KEY (t_t_id)", $sql);
				$sql = str_replace("users u", "users u USE KEY (PRIMARY)", $sql);
				break;
			case "date_asc":
				$sql .= " ORDER BY up.timestamp ASC, 
						  up.id ASC";
				// force the correct index usage
				// $sql = str_replace("offensive_uploads up", "offensive_uploads up USE KEY (t_t_id)", $sql);
				// $sql = str_replace("users u", "users u USE KEY (PRIMARY)", $sql);
				break;
			case "votes_asc":
				$sql .= " ORDER BY ca.good ASC";
				// force the correct index usage
				$sql = $after ? str_replace("offensive_uploads up", "offensive_uploads up USE KEY (t_t_id)", $sql)
				              : str_replace("offensive_uploads up", "offensive_uploads up USE KEY (status_type_id)", $sql);
				$sql = $after ? $sql : str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (good)", $sql);
				$sql = str_replace("users u", "users u USE KEY (PRIMARY)", $sql);
				break;
			case "votes_desc":
				$sql .= " ORDER BY ca.good DESC";
				// force the correct index usage
				$sql = $after ? str_replace("offensive_uploads up", "offensive_uploads up USE KEY (t_t_id)", $sql)
				              : str_replace("offensive_uploads up", "offensive_uploads up USE KEY (status_type_id)", $sql);
				$sql = $after ? $sql : str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (good)", $sql);
				$sql = str_replace("users u", "users u USE KEY (PRIMARY)", $sql);
				break;
			// these sorting options are slow due to insufficient indices.  average query time over 1s.  luckily they are uncommon.
			case "comments_asc":
				$sql .= " ORDER BY ca.comments ASC";
				$sql = str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (ca_comments)", $sql);
				break;
			case "comments_desc":
				$sql .= " ORDER BY ca.comments DESC";
				$sql = str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (ca_comments)", $sql);
				break;
			case "activity_asc":
				$sql .= " ORDER BY ca.timestamp ASC";
				$sql = str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (ca_timestamp)", $sql);
				break;
			case "activity_desc":
				$sql .= " ORDER BY ca.timestamp DESC";
				$sql = str_replace("offensive_count_cache ca", "offensive_count_cache ca USE KEY (ca_timestamp)", $sql);
				break;
			default:
				trigger_error("impossible order: $sort", E_USER_ERROR);
				exit;
		}

        global $numPerPage;
        if(!isset($numPerPage)) {
            $numPerPage = 100;
        }
        if(!array_key_exists('limit', $args)) {
            $limit = "$numPerPage";
        }
				if($p !== false) {
					$start = $p * $numPerPage;
					$limit = "$start, $numPerPage";
        }

		$sql .= " LIMIT $limit";

		$result = tmbo_query($sql);
		$ret = array();
		while($row = mysql_fetch_assoc($result)) {
			$upload = new Upload($row);
			if(!$upload->squelched()) {
				$ret[] = $upload;
			}
		}
		
		unset($result, $sql);
		
		return $ret;
	}
	
	function core_getupload($id) {
		$args = array("id" => $id);
		$id =   check_arg("id", "integer", $args);
		handle_errors();
		
		static $done = array();
		if(in_array($id, $done)) {
			return new Upload($id);
		}
		
		$upload = new Upload($id);
		
		$sql = uploadsql()." AND up.id = $id";
		$result = tmbo_query($sql);
		if(mysql_num_rows($result) == 0) {
			if(!$upload->exists()) {
				require("offensive/404.php");
			} else {
				updateCommentCount($id, 0, 0, 0, 0, 0);
			}
		} else {
			assert('mysql_num_rows($result) == 1');
			$upload = new Upload(mysql_fetch_assoc($result));
		}
		
		
		if($upload->squelched()) {
			require("offensive/403.php");
		}
		
		$done[] = $upload->id();
		
		return $upload;
	}

	function core_getyearbook($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
		
		$userid = check_arg("userid", "integer", $args, false);
		$limit =  check_arg("limit",  "limit",   $args, false);
		$sort =   check_arg("sort",   "string",  $args, false, array("date_desc", "date_asc", "uname_asc", "uname_desc"));
		$p =      check_arg("p", "integer", $args, false);
		handle_errors();
		
		$sql = uploadsql()." AND up.type = 'avatar' 
		                     AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up 
		                                  WHERE type='avatar' AND userid=u.userid)";
		
		if($userid !== false) {
			if(me()->squelched($userid)) {
				require("offensive/403.php");
			}
			
			$sql .= " AND u.userid = $userid";
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) == 0) {
				require("offensive/404.php");
			}
			
			return new Upload(mysql_fetch_assoc($result));
		}
		
		// double check date sub
		$sql .= " AND u.account_status != 'locked' AND up.timestamp > DATE_SUB(NOW(), INTERVAL 12 MONTH)";
		
		switch($sort) {
			case "date_desc":
				$sql .= " ORDER BY up.id DESC";
				break;
			case "date_asc":
				$sql .= " ORDER BY up.id ASC";
				break;
			case "uname_asc":
				$sql .= " ORDER BY u.username ASC";
				break;
			case "uname_desc":
				$sql .= " ORDER BY u.username DESC";
				break;
			case false:
				break;
			default:
				trigger_error("ASSERT: impossible order!", E_USER_ERROR);
				exit;
		}
		
		$numPerPage = 100;
		if($p !== false && $p > 0) {
			$start = $p * $numPerPage;
			$limit = "$start, $numPerPage";
		}
		
		$sql .= " LIMIT $limit";
        
		$result = tmbo_query($sql);
        
		$ret = array();
        
		if(mysql_num_rows($result) == 0)
			return $ret;
		
		while($row = mysql_fetch_assoc($result)) {
			$upload = new Upload($row);
			if(!$upload->squelched()) {
				$ret[] = $upload;
			}
		}
		
		return $ret;
	}

	function core_getcomments($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
		
		$votefilter = check_arg("votefilter", "string", $args, false);
		$userfilter = check_arg("userfilter", "integer", $args, false);
		$after = check_arg("after", "date", $args, false);
		$before = check_arg("before", "date", $args, false);
		$idmin = check_arg("idmin", "integer", $args, false);
		$idmax = check_arg("idmax", "integer", $args, false);
		$id = check_arg("id", "integer", $args, false);
		$threadmin = check_arg("threadmin", "integer", $args, false);
		$threadmax = check_arg("threadmax", "integer", $args, false);
		$thread = check_arg("thread", "integer", $args, false);
		$sort = check_arg("sort", "string", $args, false, array("date_desc", "date_asc"));
		$limit = check_arg("limit", "limit", $args, false);
		$userid = check_arg("userid", "integer", $args, false);
		handle_errors();
		
		$sql = commentsql();
		
		if(strpos($votefilter, "+") !== false) {
			$sql .= " AND com.vote = 'this is good'";
		}
		if(strpos($votefilter, "-") !== false) {
			$sql .= " AND com.vote = 'this is bad'";
		}
		if(strpos($votefilter, "0") !== false) {
			$sql .= " AND com.vote = ''";
		}
		if(strpos($votefilter, "x") !== false) {
			$sql .= " AND com.offensive = 1";
		}
		if(strpos($votefilter, "r") !== false) {
			$sql .= " AND com.repost = 1";
		}
		if(strpos($votefilter, "c") !== false) {
			$sql .= " AND com.comment != ''";
		}
		
		if($userfilter !== false) {
			// XXX: deprecate
			$sql .= " AND com.userid = $userfilter";
		}
		if($after !== false) {
			$sql .= " AND com.timestamp > '$after'";
		}
		if($before !== false) {
			$sql .= " AND com.timestamp < '$before'";
		}
		if($idmin !== false) {
			$sql .= " AND com.id >= $idmin";
		}
		if($idmax !== false) {
			$sql .= " AND com.id <= $idmax";
		}
		if($id !== false) {
			$sql .= " AND com.id = $id";
		}
		if($threadmin !== false) {
			$sql .= " AND com.fileid >= $threadmin";
		}
		if($threadmax !== false) {
			$sql .= " AND com.fileid <= $threadmax";
		}
		if($thread !== false) {
			$sql .= " AND com.fileid = $thread";
		}
		if($userid !== false) {
			$sql .= " AND com.userid = $userid";
		}
		
		if($sort !== false) {
			switch($sort) {
				case "date_desc":
					$sql .= " ORDER BY id DESC";
					break;
				case "date_asc":
					$sql .= " ORDER BY id ASC";
					break;
				default:
					trigger_error("ASSERT: impossible order!", E_USER_ERROR);
					exit;
			}
		}
		
		$sql .= " LIMIT $limit";
		
		$result = tmbo_query($sql);
		
		$ret = array();
		
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new Comment($row);
		}
		
		return $ret;
	}
	
	function core_getcomment($id) {
		$singleton = false;
		if(!is_array($id)) {
			$singleton = true;
			$id = array("id" => $id);
		}
		
		foreach($id as $key => $arg) {
			check_arg($key, "integer", $id);
		}
		handle_errors();

		$sql = commentsql();

		if(count($id) == 1) {
			$sql .= " AND com.id = ".$id[0];
		} else {
			$sql .= " AND ( com.id = ".$id[0];
			for($i = 1; $i < count($id); $i++) {
				$sql .= " OR com.id = ".$id[$i];
			}
			$sql .= ")";
		}

		$result = tmbo_query($sql);
		if(mysql_num_rows($result) == 0 && $singleton) {
			require("offensive/404.php");
		}

		$ret = array();
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new Comment($row);
		}
		if($singleton) {
			assert('count($ret) == 1');
		}

		return $singleton ? array_pop($ret) : $ret;
	}

	function core_postcomment($args) {
		$fileid = check_arg("fileid", "integer", $args);
		$comment = check_arg("comment", "string", $args, false);
		$vote = check_arg("vote", "string", $args, false, array("this is good", "this is bad", "novote"));
		$offensive = check_arg("offensive", "integer", $args, false, array("1", "0"));
		$repost = check_arg("repost", "integer", $args, false, array("1", "0"));
		$subscribe = check_arg("subscribe", "integer", $args, false, array("1", "0"));
		handle_errors();
		
		$upload = new Upload($fileid);
		if(!$upload->exists()) return false; // error

		$userid = me() ? me()->id() : null;
		if($userid == null) return false; // error
		
		if($vote == "novote") $vote = "";
		if($comment === false) $comment = "";
		if($vote === false) $vote = "";
		if($offensive === false) $offensive = 0;
		if($repost === false) $repost = 0;
		
		// if no comment, vote, offensive, or repost, then why are you here?
		if(!($comment || $vote || $offensive || $repost || $subscribe)) {
			send(false);
		}

		$comment = trim($comment);

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
			return false; // error
		}

		// prevent empty comments from being inserted
		if(!($comment || $vote || $offensive || $repost)) {
			return false; // error
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
			return false; // error
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

		return new Comment($commentid);
	}
	
	// XXX: unimplemented
	function core_posttopic($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
			
		$title = check_arg("title", "string", $args);
		$comment = check_arg("comment", "string", $args, false);
		handle_errors();
		
		// XXX: c'moon :(
		trigger_error("unimplemented", E_USER_ERROR);
	}

	// XXX: unimplemented (broken)
	function core_searchcomments($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
			
		$q = check_arg("q", "string", $method);
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		trigger_error("not implemented :(", E_USER_ERROR);
	}

	function core_searchuser($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}

		$limit = check_arg("limit", "limit", $args, false);
		$q = check_arg("q", "string", $args);
		handle_errors();
		
		$q = sqlEscape($q);
		
		$sql = "SELECT * FROM users WHERE username LIKE '%$q%' LIMIT $limit";
		
		$result = tmbo_query($sql);
		
		$ret = array();
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new User($row);
		}
		return $ret;
	}

	function core_searchuploads($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
					
		$q = check_arg("q", "string", $args);
		$limit = check_arg("limit", "limit", $args, false);
		$type = check_arg("type", "string", $args, false, array("image", "topic", "avatar", "audio"));
		handle_errors();
		
		$sql = uploadsql();
		if($type !== false) {
			$sql .= " AND up.type = '$type'";
		}
		$sql .= " AND up.filename LIKE '%".sqlEscape($q)."%' ORDER BY up.timestamp DESC LIMIT $limit";
		
		$result = tmbo_query($sql);
		
		$ret = array();
		
		while($row = mysql_fetch_assoc($result)) {
			$upload = new Upload($row);
			if(!$upload->squelched()) {
				$ret[] = $upload;
			}
		}
		
		return $ret;
	}

	function core_unreadcomments($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}

		$sort = check_arg("sort", "string", $args, false, array("comment_desc", "comment_asc", "file_asc", "file_desc"));
		$limit = check_arg("limit", "limit", $args, false);
		handle_errors();

		if($sort === false) $sort = "file_asc";

		$sql = commentsql();

		if(!me()) {
			trigger_error("must be logged in to use this function", E_USER_ERROR);
		}
		$userid = me()->id();

		$sql = str_replace("FROM offensive_comments com", 
		                   "FROM offensive_subscriptions sub, offensive_comments com", $sql);
		$sql .= " AND sub.userid = $userid AND up.id = sub.fileid AND sub.commentid = com.id ORDER BY ";

		switch($sort) {
			case "comment_desc":
				$sql .= "sub.commentid DESC";
				break;
			case "comment_asc":
				$sql .= "sub.commentid ASC";
				break;
			case "file_desc":
				$sql .= "up.id DESC";
				break;
			case "file_asc":
				$sql .= "up.id ASC";
				break;
			default:
				trigger_error("ASSERT: impossible order!", E_USER_ERROR);
				exit;
		}

		$sql .= " LIMIT $limit";

		$result = tmbo_query($sql);

		$ret = array();

		while($row = mysql_fetch_assoc($result)) {
			$comment = new Comment($row);
			if(!$comment->upload()->squelched()) {
				$ret[] = $comment;
			}
		}

		return $ret;
	}

	function core_getadmins() {
		$sql = "SELECT * FROM users WHERE account_status = 'admin'";
		$result = tmbo_query($sql);
		$ret = array();
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new User($row);
		}
		return $ret;
	}

	function core_createtoken($issuee) {
		if(!is_string($issuee)) {
			trigger_error("issuee is not a string", E_USER_ERROR);
		}
		
		if($issuee == " tmbo") {
			/*
			 * using new Token() here creates a possible infinite loop if no default
			 * token exists, because the Token class calls back to core_createtoken
			 * to create a token if a default token does not already exist.
			 * 
			 * this method is slightly slower, but it's uncommon to create a new
			 * token for " tmbo", and we want to be paranoid about duplicates for
			 * this special case.
			 */
			$tokens = core_gettokens();
			foreach($tokens as $token) {
				if($token->issued_to() == " tmbo") {
					trigger_error("token already exists for user, using existing token ".$token->tokenid(), E_USER_WARNING);
					return $token;
				}
			}
		}
		
		do {
			$tokenid = "";
			for($i = 0; $i < 32; $i++) {
				$tokenid .= base_convert(rand(0, 35), 10, 36);
			}
			$dupcheck = new Token($tokenid);
		} while($dupcheck->exists());
		unset($dupcheck);
		
		$sql = "INSERT INTO tokens (tokenid, userid, issued_to, issue_date) VALUES (\"$tokenid\", ".me()->id().", \"".sqlEscape($issuee)."\", NOW())";
		
		tmbo_query($sql);
		
		$token = new Token($tokenid);
		$token->reset_validity();
		
		return $token;
	}
	
	function core_gettokens() {
		$sql = "SELECT * FROM tokens WHERE userid = ".me()->id();
		$result = tmbo_query($sql);

		$ret = array();

		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new Token($row);
		}

		return $ret;
	}
	
	function core_destroytoken($token) {
		if(is_string($token)) {
			$token = new Token($token);
			if(!$token->exists()) {
				trigger_error("token is not valid", E_USER_ERROR);
			}
		}
		
		if(!is_object($token) || $token->exists() == false) {
			trigger_error("token is not valid", E_USER_ERROR);
		}
		if($token->userid() != me()->id()) {
			trigger_error("token does not belong to the current user", E_USER_ERROR);
		}
		
		$sql = "DELETE FROM tokens WHERE tokenid = \"".$token->id()."\"";
		tmbo_query($sql);
	}

	function core_getlocation($args) {
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
		if(!me()) array();
		
		$userid =  check_arg("userid",  "integer", $args, false);
		$minlat =  check_arg("minlat",  "float",   $args, false);
		$maxlat =  check_arg("maxlat",  "float",   $args, false);
		$minlong = check_arg("minlong", "float",   $args, false);
		$maxlong = check_arg("maxlong", "float",   $args, false);
		$limit =   check_arg("limit",   "limit",   $args, false);
		handle_errors();
		
		$sql = "SELECT loc.x as latitude, loc.y as longitude,
		               loc.timestamp, u.username, loc.userid
		        FROM maxxer_locations loc, users u 
				    WHERE u.userid = loc.userid
				    AND loc.mapversion = 'google'
	          AND u.account_status != 'locked'
	          AND u.timestamp > DATE_SUB( NOW(), INTERVAL 12 MONTH )";

		if($userid) {
			$sql .= " AND loc.userid = $userid";
		}
		if($minlat) {
			$sql .= " AND loc.x >= $minlat";
		}
		if($maxlat) {
			$sql .= " AND loc.x <= $maxlat";
		}
		if($minlong) {
			$sql .= " AND loc.y >= $minlong";
		}
		if($maxlong) {
			$sql .= " AND loc.y <= $maxlong";
		}
		
		$sql .= " ORDER BY timestamp DESC LIMIT $limit";
		
		$result = tmbo_query($sql);
		
		$rows = array();
		while($row = mysql_fetch_assoc($result)) {
			if(!me()->squelched($row["userid"])) {
				$rows[] = $row;
			}
		}
		
		return $rows;
	}

?>
