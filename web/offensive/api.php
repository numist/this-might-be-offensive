<?
	if(!isset($_SERVER["HTTPS"]) || $_SERVER["HTTPS"] != "on") {
		header("Location: https://".$_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"], 301);
		exit;
	}

	set_include_path("..");

	require_once("offensive/assets/argvalidation.inc");
		
	// some output types (notably xml) like to know what kinds
	// of objects you're returning (upload, user, comment, etc)
	$objects = null;

	// get function name and return type
	$call = array_pop(explode("/", $_SERVER["REQUEST_URI"]));
	list($func, $rtype) = explode(".", $call);
	if(strpos($rtype, "?") !== false) {
		$rtype = array_shift(explode("?", $rtype));
	}
	
	// return type validation and definitions:
	function php_encode($data) { return serialize($data); }
	require_once("offensive/assets/plist.inc");
	require_once("offensive/assets/xml.inc");
	$rtype = strtolower($rtype);
	if(!is_callable($rtype."_encode")) {
		header("HTTP/1.0 400 Bad Request");
		header("Content-type: text/plain");
		echo "unsupported return format $rtype.";
		exit;
	}
	
	// validate the function call is valid
	if(!is_callable("api_".$func)) {
		header("HTTP/1.0 404 Not Found");
		header("Content-type: text/plain");
		echo "the function you requested ($func) was not found on this server.";
		exit;
	}

	// set up the normal TMBO environment
	require_once( 'offensive/assets/header.inc' );
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	
	// authentication
	if($func != "login" && !loggedin(false)) {
		mustLogIn("http");
	}
	
	// standardized sql query for getting uploads from the db. see: api_getupload(s)?
	$userid = $_SESSION['userid'];
	$uploadsql = "SELECT up.id, up.userid, up.filename, up.timestamp, up.nsfw, up.tmbo, up.type, u.username,
		(SELECT COUNT(*) FROM offensive_subscriptions WHERE userid = $userid AND fileid = up.id) as subscribed,
		ca.good as vote_good, ca.bad as vote_bad, ca.tmbo as vote_tmbo, ca.repost as vote_repost, ca.comments
	FROM offensive_uploads up, offensive_count_cache ca, users u
	WHERE ca.threadid = up.id AND u.userid = up.userid AND up.status = 'normal'";
	
	$commentsql = "SELECT com.fileid, com.id, com.comment, com.vote, com.offensive, com.repost, com.timestamp, u.userid, u.username FROM offensive_comments com, users u WHERE com.userid = u.userid";

/**
	Helper functions.
**/

	/*
	 * coerce values of an array from strings to the appropriate PHP types.
	 */
	function format_data(&$data) {
		global $rtype;
		
		if(is_array($data)) {
			foreach($data as $key => $val) {
				format_data($data[$key]);
			}
		/* plists get a special date format as their output,
		 * due to spec restrictions.
		 */
		} else if(strtotime($data) > 0 && $rtype == "plist") {
			$data = date('c', strtotime($data));
		} else if(is_numeric($data)) {
			if(strpos($data, ".") === false) {
				$data = (int)$data;
			} else {
				$data = (double)$data;
			}
		} else if($data == "true") {
			$data = true;
		} else if($data == "false") {
			$data = false;
		}
	}
	
	/*
	 * return php data to the caller in the format requested.
	 */
	require_once("offensive/assets/conditionalGet.inc");
	function send($ret) {
		global $rtype;
		
		// get the newest timestamp in the dataset (if possible) and conditional GET
		if(is_array($ret)) {
			if(array_key_exists("timestamp", $ret)) {
				conditionalGet($ret['timestamp']);
			} else {
				$timestamp = 0;
				foreach($ret as $val) {
					if(is_array($val) && array_key_exists("timestamp", $val) && 
					   // this line is silly, but it avoids an extra call to strtotime.
					   ($tmpstamp = strtotime($val['timestamp'])) > $timestamp) {
						$timestamp = $tmpstamp;
					}
				}
				if($timestamp > 0) {
					conditionalGet($timestamp);
				}
			}
		}
		
		// send the data back
		if(in_array($rtype, array("plist", "xml"))) {
			header("Content-type: text/xml");
		} else {
			header("Content-type: text/plain");
		}
		echo call_user_func($rtype."_encode", $ret);
		exit;
	}
	
	/*
	 * run a query and return all rows in an array, even if there are 0.
	 */
	function get_rows($sql) {
		$rows = array();
		$result = tmbo_query($sql);
		if(mysql_num_rows($result) == 0) {
			return false;
		}
		while(false !== ($row = mysql_fetch_assoc($result))) {
				$rows[] = $row;
		}
		
		format_data($rows);
		
		return $rows;
	}
	
	/*
	 * run a query and return a single row.
	 * return false if there are no results.
	 */
	function get_row($sql) {
		$result = tmbo_query($sql);
		if(mysql_num_rows($result) == 0) {
			return false;
		}
		
		$ret = mysql_fetch_assoc($result);
		format_data($ret);
		return $ret;
	}
	
	/*
	 * Called on each row returned from a query based on $uploadsql.
	 * Adds elements for file link, file dimensions, thumb link, and
	 * thumb dimensions, if applicable.
	 * Also coerces subscribed, nsfw, and tmbo keys to booleans.
	 * If the row is expected to return next and previous pointers, they are added.
	 */
	function format_uprow(&$row, $neighbours = true) {		
		// edge cases:  subscribed should be boolean.  yay or nay
		$row['subscribed'] = $row['subscribed'] == 0 ? false : true;
		// tmbo and nsfw should also be boolean.  and not null.
		$row['nsfw'] = $row['nsfw'] == "1" ? true : false;
		$row['tmbo'] = $row['tmbo'] == "1" ? true : false;
		
		$filename = getFile($row['id'], $row['filename'], $row['timestamp'], $row['type']);
		if($filename == "") {
			$row['link_file'] = false;
		} else {
			$row['link_file'] = getFileURL($row['id'], $row['filename'], $row['timestamp'], $row['type']);
			$size = getimagesize($filename);
			if(is_array($size)) {
				$row['width'] = $size[0];
				$row['height'] = $size[1];
			}
		}
		$thumb = getThumb($row['id'], $row['filename'], $row['timestamp'], $row['type']);
		if($thumb == "") {
			$row['link_thumb'] = false;
		} else {
			$row['link_thumb'] = getThumbURL($row['id'], $row['filename'], $row['timestamp'], $row['type']);
			$size = getimagesize($thumb);
			if(is_array($size)) {
				$row['thumb_width'] = $size[0];
				$row['thumb_height'] = $size[1];
			}
		}
		
		$upload = $row['id'];
		// if type is topic or image, get the next in line as well.
		if(($row['type'] == "image" || $row['type'] == "topic") && $neighbours) {
			$sql = "SELECT id FROM offensive_uploads WHERE type = '".$row['type']."' AND id > $upload AND status = 'normal' ORDER BY id ASC LIMIT 1";
			$next = get_row($sql);
			if(is_array($next)) {
				$row['next'] = $next['id'];
			}
			
			$sql = "SELECT id FROM offensive_uploads WHERE type = '".$row['type']."' AND id < $upload AND status = 'normal' ORDER BY id DESC LIMIT 1";
			$prev = get_row($sql);
			if(is_array($prev)) {
				$row['prev'] = $prev['id'];
			}
		}
	}
	
	function format_commentrows(&$rows) {
		for($i = 0; $i < count($rows); $i++) {
			if($rows[$i]['vote'] == null) unset($rows[$i]['vote']);
			if($rows[$i]['offensive'] == null) unset($rows[$i]['offensive']);
			if($rows[$i]['repost'] == null) unset($rows[$i]['repost']);
		}
	}
	
/**
	API functions
**/	
	call_user_func("api_".$func);

	function api_getuploads($args=null) {
		global $uploadsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		$type = check_arg("type", "string", $method, false, array("image", "topic", "avatar"));
		$userid = check_arg("userid", "integer", $method, false);
		$after = check_arg("after", "date", $method, false);
		$before = check_arg("before", "date", $method, false);
		$max = check_arg("max", "integer", $method, false);
		$since = check_arg("since", "integer", $method, false);
		$sort = check_arg("sort", "string", $method, false, array("date_desc", "date_asc", "votes_asc", "votes_desc"));
		$limit = check_arg("limit", "limit", $method, false);
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
		switch($sort) {
			case "date_desc":
				$sql .= " ORDER BY up.id DESC";
				break;
			case "date_asc":
				$sql .= " ORDER BY up.id ASC";
				break;
			case "votes_asc":
				$sql .= " ORDER BY ca.good ASC";
				break;
			case "votes_desc":
				$sql .= " ORDER BY ca.good DESC";
				break;
			default:
				trigger_error("ASSERT: impossible order!", E_USER_ERROR);
				exit;
		}
		
		$sql .= " LIMIT $limit";
		
		$rows = get_rows($sql);
		
		for($i = 0; $i < count($rows); $i++) {
			format_uprow($rows[$i], false);
		}
		
		global $objects; $objects = $type ? $type : "upload";
		send($rows);
	}
	
	function api_getupload($args=null) {
		global $uploadsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		$upload = check_arg("upload", "integer", $method);
		handle_errors();
		
		$sql = $uploadsql;
		
		$sql .= " AND up.id = $upload LIMIT 1";
		
		$row = get_row($sql);
		
		if($row === false) send(false);
		
		format_uprow($row);
		
		send($row);
	}
	
	function api_getyearbook($args=null) {
		global $uploadsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		$userid = check_arg("userid", "integer", $method, false);
		$limit = check_arg("limit", "limit", $method, false);
		$sort = check_arg("sort", "string", $method, false, array("date_desc", "date_asc", "uname_asc", "uname_desc"));
		handle_errors();
		
		$sql = $uploadsql." AND up.type = 'avatar' AND up.id = (SELECT MAX(upl.id) FROM offensive_uploads upl WHERE upl.type='avatar' AND upl.userid=u.userid)";
		
		if($userid !== false) {
			$sql .= " AND u.userid = $userid";
			
			$row = get_row($sql);
			
			format_uprow($row);

			send($row);
		} else {
			$sql .= " AND u.account_status != 'locked'";
			
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
			$sql .= " LIMIT $limit";
			
			$rows = get_rows($sql);
			
			for($i = 0; $i < count($rows); $i++) {
				format_uprow($rows[$i]);
			}
			
			global $objects; $objects = "avatar";
			
			send($rows);
		}
	}
	
	function api_getuser($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$userid = check_arg("userid", "integer", $method, false);
		handle_errors();

		if(!$userid) {
			$userid = $_SESSION['userid'];
		}
		
		$sql = "SELECT (SELECT COUNT(*) FROM users WHERE referred_by = $userid AND userid != $userid) as posse, (SELECT COUNT(*) FROM offensive_uploads WHERE type = 'avatar' AND userid = $userid) as yearbook, userid, username, created, account_status, timestamp, referred_by";
		if($_SESSION['status'] == "admin") {
			$sql .= ", email, last_login_ip";
		}
		$sql .= " FROM users WHERE userid = $userid";

		send(get_row($sql));
	}
	
	function api_getposse($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		check_arg("userid", "integer", $method, false);
		handle_errors();
		
		if(isset($_REQUEST['userid'])) {
			$userid = $_REQUEST['userid'];
		} else {
			$userid = $_SESSION['userid'];
		}
		
		$sql = "SELECT (SELECT COUNT(*) FROM users u WHERE referred_by = users.userid AND u.userid != users.userid) as posse, (SELECT COUNT(*) FROM offensive_uploads WHERE type = 'avatar' AND userid = users.userid) as yearbook, userid, username, created, account_status, timestamp, referred_by";
		if($_SESSION['status'] == "admin") {
			$sql .= ", email, last_login_ip";
		}
		$sql .= " FROM users WHERE referred_by = $userid AND userid != $userid";
		
		global $objects; $objects = "user";
		
		send(get_rows($sql));
	}
	
	function api_login($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		check_arg("username", "string", $method);
		check_arg("password", "string", $method);
		handle_errors();
		session_unset();
		
		$loggedin = login($_REQUEST['username'], $_REQUEST['password']);
		if($loggedin === false) {
			global $login_message;
			header("HTTP/1.0 401 Unauthorized");
			echo $login_message;
			exit;
		} else if($loggedin === null) {
			global $login_message;
			header("HTTP/1.0 403 Forbidden");
			echo $login_message;
			exit;
		}
		$_REQUEST['userid'] = $_SESSION['userid'];
		api_getuser();
	}
	
	function api_logout($args=null) {
		session_unset();
		send(true);
	}
	
	function api_getcomments($args=null) {
		global $commentsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		$votefilter = check_arg("votefilter", "string", $method, false);
		$userfilter = check_arg("userfilter", "integer", $method, false);
		$after = check_arg("after", "date", $method, false);
		$before = check_arg("before", "date", $method, false);
		$idmin = check_arg("idmin", "integer", $method, false);
		$idmax = check_arg("idmax", "integer", $method, false);
		$id = check_arg("id", "integer", $method, false);
		$threadmin = check_arg("threadmin", "integer", $method, false);
		$threadmax = check_arg("threadmax", "integer", $method, false);
		$thread = check_arg("thread", "integer", $method, false);
		$sort = check_arg("sort", "string", $method, false, array("date_desc", "date_asc"));
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		$sql = $commentsql;
		
		if(strpos($votefilter, "+") !== false) {
			$sql .= " AND com.vote = 'this is good'";
		}
		if(strpos($votefilter, "-") !== false) {
			$sql .= " AND com.vote = 'this is bad'";
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
			// XXX: reset subscription!
			$sql .= " AND com.fileid = $thread";
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
		
		$rows = get_rows($sql);
		
		format_commentrows($rows);

		global $objects; $objects = "comment";
		
		send($rows);
	}
	
	function api_postcomment($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_POST;
			
		$fileid = check_arg("fileid", "integer", $methd);
		$comment = check_arg("comment", "string", $method, false);
		$vote = check_arg("vote", "string", $method, false, array("this is good", "this is bad"));
		$offensive = check_arg("offensive", "integer", $method, false, array("1", "0"));
		$repost = check_arg("repost", "integer", $method, false, array("1", "0"));
		handle_errors();
		
		send(false);
	}
	
	// XXX: to upload a file from within, do not call this function directly!
	function api_postupload($args=null) {
		$type = check_arg("type", "string", null, true, array("avatar", "image"));
		// FIX THIS TO WORK RIGHT!
		check_arg("filename", "string", $_FILE);
		$filename = check_arg("filename", "string", $_POST, false);
		$comment = check_arg("comment", "string", $_POST);
		$nsfw = check_arg("nsfw", "integer", $_POST, false, array("0", "1"));
		$tmbo = check_arg("tmbo", "integer", $_POST, false, array("0", "1"));
		handle_errors();
		
		// XXX: this will have to call some uploader helper functions to cooperate with the upload page.
		
		send(false);
	}
	
	function api_posttopic($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_POST;
			
		$title = check_arg("title", "string", $method);
		$comment = check_arg("comment", "string", $method, false);
		handle_errors();
		
		send(false);
	}
	
	function api_searchcomments($args=null) {
		global $commentsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$q = check_arg("q", "string", $method);
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		$sql = $commentsql." AND MATCH(com.comment) AGAINST('".sqlEscape($q)."' IN BOOLEAN MODE)
				ORDER BY com.id LIMIT $limit";
		
		$rows = get_rows($sql);
		
		format_commentrows($rows);
		
		global $objects; $objects = "comment";
		
		send($rows);
	}
	
	function api_searchuser($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$q = check_arg("q", "string", $method);
		handle_errors();
		$q = sqlEscape($q);
		
		$sql = "SELECT userid FROM users WHERE username = '$q' LIMIT 1";
		$row = get_row($sql);
		if($row === false) send(false);

		api_getuser(array('userid' => $row['userid']));
	}
	
	function api_searchuploads($args=null) {
		global $uploadsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$q = check_arg("q", "string", $method);
		$limit = check_arg("limit", "limit", $method, false);
		$type = check_arg("type", "string", $method, false, array("image", "topic", "avatar"));
		handle_errors();
		
		$sql = $uploadsql;
		if($type !== false) {
			$sql .= " AND up.type = '$type'";
		}
		$sql .= " AND up.filename LIKE '%".sqlEscape($q)."%' ORDER BY up.timestamp DESC LIMIT $limit";
		
		$rows = get_rows($sql);
		for($i = 0; $i < count($rows); $i++) {
			format_uprow($rows[$i], false);
		}
		
		global $objects;
		$objects = $type ? $type : "upload";
		
		send($rows);
	}
	
	function api_invite($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$email = check_arg("email", "string", $method);
		handle_errors();
		
		send(false);
	}
	
	function api_faq($args=null) {
		send("<ul><li>Don't be retarded.</li></ul>");
	}
	
	function api_getlocation($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$userid = check_arg("userid", "integer", $method, false);
		$minlat = check_arg("minlat", "float", $method, false);
		$maxlat = check_arg("maxlat", "float", $method, false);
		$minlong = check_arg("minlong", "float", $method, false);
		$maxlong = check_arg("maxlong", "float", $method, false);
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		$sql = "SELECT loc.x as latitude, loc.y as longitude, loc.timestamp, u.username, loc.userid
		        FROM maxxer_locations loc, users u 
				WHERE u.userid = loc.userid";

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
		
		$rows = get_rows($sql);
		
		global $objects; $objects = "location";
		
		send($rows);
	}

	function api_setlocation($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$lat = check_arg("lat", "float", $method);
		$long = check_arg("long", "float", $method);
		$userid = $_SESSION['userid'];
		handle_errors();
		
		$sql = "REPLACE INTO maxxer_locations (userid, x, y, mapversion) VALUES( $userid, $lat, $long, 'google' )";
		$result = tmbo_query( $sql );
		send(true);
	}
	
	function api_pickupid($args=null) {
		send(false);
	}
	
	
	function api_unreadcomments($args=null) {
		global $uploadsql;
		
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
		
		$sort = check_arg("sort", "string", $method, false, array("comment_desc", "comment_asc", "file_asc", "file_desc"));
		$limit = check_arg("limit", "limit", $method, false);
		handle_errors();
		
		if($sort === false) $sort = "file_asc";
		
		$sql = $uploadsql;
		$userid = $_SESSION['userid'];
		
		/*
		 * because we want the uploadsql basic query, but with an additional column and table, we have to
		 * monkeypatch the sql string.  the matching term is so long to prevent matching the nested SELECT
		 * in the uploadsql query.
		 */
		$sql = str_replace("FROM offensive_uploads up", 
		                   ", sub.commentid FROM offensive_subscriptions sub, offensive_uploads up", $sql);
		
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
		
		$rows = get_rows($sql);
		for($i = 0; $i < count($rows); $i++) {
			format_uprow($rows[$i], false);
		}
		
		global $objects; $objects = "upload";
		
		send($rows);
	}
	
	
	function api_subscribe($args=null) {
		if(is_array($args))
			$method = $args;
		else
			$method = $_REQUEST;
			
		$threadid = check_arg("threadid", "integer", $method);
		$subscribe = check_arg("subscribe", "integer", $method, false, array("1", "0"));
		handle_errors();
		
		if($subscribe === false) $subscribe = 1;
		$userid = $_SESSION['userid'];
		
		if($subscribe == 1) {
			$sql = "SELECT * FROM offensive_subscriptions WHERE userid = $user AND fileid = $threadid";
			if(mysql_num_rows(tmbo_query($sql)) > 0) {
				send(true);
			}
			$sql = "SELECT * FROM offensive_uploads WHERE id = $threadid";
			if(mysql_num_rows(tmbo_query($sql)) == 0) {
				send(false);
			}

			$sql = "INSERT INTO offensive_subscriptions (userid, fileid) VALUES ( $userid, $threadid )";
			tmbo_query( $sql );
			send(true);
		}
		
		$sql = "DELETE FROM offensive_subscriptions WHERE userid=$userid AND fileid=$threadid";
		tmbo_query( $sql );
		send(true);
	}

?>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
