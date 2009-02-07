<?
	// XXX: check that we actually need all these
	require_once( 'offensive/assets/header.inc' );
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/classes.inc");
	
	require_once("offensive/assets/argvalidation.inc");
	
	// standardized sql query for getting uploads from the db. see: api_getupload(s)?
	$subscribed = array_key_exists('userid', $_SESSION)
	              ? "("."SELECT COUNT(*) FROM offensive_subscriptions 
	                     WHERE userid = {$_SESSION['userid']} AND fileid = up.id".")"
	              : "NULL";
	
	$uploadsql = "SELECT up.*,
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
	WHERE ca.threadid = up.id AND up.userid = u.userid";
	
	$commentsql = "SELECT com.*,
	                     u.username as user_username, u.account_status as user_account_status, u.userid as user_userid,
	              		 up.userid as upload_userid, up.id as upload_id, up.filename as upload_filename,
	                     up.timestamp as upload_timestamp, up.nsfw as upload_nsfw, up.tmbo as upload_tmbo,
	                     up.type as upload_type, up.status as upload_status
	               FROM offensive_comments com, users u, offensive_uploads up
	               WHERE com.userid = u.userid AND up.id = com.fileid";
	
	require_once("offensive/assets/comments.inc");

/**
	core functions
**/	
	global $types;
	$types = array("image", "topic", "avatar");

	function core_getuploads($args) {
		global $uploadsql, $types;
		
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}

		// XXX: need a more fatal way to fail at this level, since we're core.
		// YYY: API should be soft-fail.
		$type =   check_arg("type",   "string",  $args, false, $types);
		$userid = check_arg("userid", "integer", $args, false);
		$after =  check_arg("after",  "date",    $args, false);
		$before = check_arg("before", "date",    $args, false);
		$max =    check_arg("max",    "integer", $args, false);
		$since =  check_arg("since",  "integer", $args, false);
		$sort =   check_arg("sort",   "string",  $args, false, array("date_desc", "date_asc", "votes_asc", "votes_desc", "comments_asc", "comments_desc", "activity_asc", "activity_desc"));
		$limit =  check_arg("limit",  "limit",   $args, false);
		$p =      check_arg("p", "integer", $args, false);
		handle_errors();
		
		// sort order needs to always be set, even if only to default.
		if($sort === false) $sort = "date_desc";
		
		$sql = $uploadsql;
		
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
			$sql .= " AND up.id >= $since";
		}
		$sql .= " AND up.status = 'normal'";
		switch($sort) {
			// XXX: date orders have two order clauses because...  well because otherwise the query takes half a minute D:
			case "date_desc":
				$sql .= " ORDER BY up.timestamp DESC, 
						  up.id DESC";
				break;
			case "date_asc":
				$sql .= " ORDER BY up.timestamp ASC, 
						  up.id ASC";
				break;
			case "votes_asc":
				$sql .= " ORDER BY ca.good ASC";
				break;
			case "votes_desc":
				$sql .= " ORDER BY ca.good DESC";
				break;
			case "comments_asc":
				$sql .= " ORDER BY ca.comments ASC";
				break;
			case "comments_desc":
				$sql .= " ORDER BY ca.comments DESC";
				break;
			case "activity_asc":
				$sql .= " ORDER BY ca.timestamp ASC";
				break;
			case "activity_desc":
				$sql .= " ORDER BY ca.timestamp DESC";
				break;
			default:
				trigger_error("impossible order: $sort", E_USER_ERROR);
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
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new Upload($row);
		}
		
		unset($result, $sql);
		
		return $ret;
	}

	function core_getyearbook($args) {
		global $uploadsql;
		
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
		
		$userid = check_arg("userid", "integer", $args, false);
		$limit =  check_arg("limit",  "limit",   $args, false);
		$sort =   check_arg("sort",   "string",  $args, false, array("date_desc", "date_asc", "uname_asc", "uname_desc"));
		handle_errors();
		$p =      check_arg("p", "integer", $args, false);
		
		$sql = $uploadsql." AND up.type = 'avatar' 
		                    AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up 
		                                 WHERE type='avatar' AND userid=u.userid)";
		
		if($userid !== false) {
			$sql .= " AND u.userid = $userid";
			
			$result = tmbo_query($sql);
			
			if(mysql_num_rows($result) == 0)
				return null;
			
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
			$ret[] = new Upload($row);
		}
		
		return $ret;
	}

	function core_getcomments($args) {
		global $commentsql;
		
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
		
		$sql = $commentsql;
		
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
			// XXX: reset subscription?
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
		global $commentsql;
		
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
					
		$q = check_arg("q", "string", $method);
		handle_errors();
		
		$q = sqlEscape($q);
		
		$sql = "SELECT * FROM users WHERE username LIKE '%$q%'";
		
		$result = tmbo_query($sql);
		
		$ret = array();
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new User($row);
		}
		return $ret;
	}

	function core_searchuploads($args) {
		global $uploadsql;
		
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
					
		$q = check_arg("q", "string", $method);
		$limit = check_arg("limit", "limit", $method, false);
		$type = check_arg("type", "string", $method, false, array("image", "topic", "avatar"));
		handle_errors();
		
		$sql = $uploadsql;
		if($type !== false) {
			$sql .= " AND up.type = '$type'";
		}
		$sql .= " AND up.filename LIKE '%".sqlEscape($q)."%' ORDER BY up.timestamp DESC LIMIT $limit";
		
		$result = tmbo_query($sql);
		
		$ret = array();
		
		while($row = mysql_fetch_assoc($result)) {
			$ret[] = new Upload($row);
		}
		
		return $ret;
	}

	function core_unreadcomments($args) {
		global $commentsql;
		
		if(!is_array($args)) {
			trigger_error("args is not an array", E_USER_ERROR);
		}
		
		$sort = check_arg("sort", "string", $method, false, array("comment_desc", "comment_asc", "file_asc", "file_desc"));
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		if($sort === false) $sort = "file_asc";
		
		$sql = $commentsql;
		
		if(!isset($_SESSION) || !array_key_exists("userid", $_SESSION)) {
			trigger_error("must be logged in to use this function", E_USER_ERROR);
		}
		$userid = $_SESSION['userid'];
		
		/*	$commentsql = "SELECT com.*,
		             u.username as user_username, u.account_status as user_account_status, u.userid as user_userid,
		        	 up.userid as upload_userid, up.id as upload_id, up.filename as upload_filename,
		             up.timestamp as upload_timestamp, up.nsfw as upload_nsfw, up.tmbo as upload_tmbo,
		             up.type as upload_type, up.status as upload_status
		       FROM offensive_comments com, users u, offensive_uploads up
		       WHERE com.userid = u.userid AND up.id = com.fileid";
		*/
		$sql = str_replace("FROM offensive_comments com", 
		                   "FROM offensive_subscriptions sub, offensive_comments com", $sql);
		$sql .= " AND sub.userid = $userid AND up.id = sub.fileid AND sub.commentid IS NOT NULL ORDER BY ";
		
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
			$ret[] = new Comment($row);
		}
		
		return $ret;
	}
	
?>