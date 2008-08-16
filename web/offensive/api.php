<?
	set_include_path("..");
		
	$errors = array();

	// get function name and return type
	$call = array_pop(explode("/", $_SERVER["REQUEST_URI"]));
	list($func, $rtype) = explode(".", $call);
	if(strpos($rtype, "?") !== false) {
		$rtype = array_shift(explode("?", $rtype));
	}
	
	// validate the return type
	$rtype = strtolower($rtype);
	if(!is_callable($rtype."_encode") ||
	   ($rtype != 'json' && $rtype != 'plist')) {
		header("HTTP/1.0 400 Bad Request");
		echo "unsupported return format $rtype.";
		exit;
	}
	require_once("offensive/assets/plist.inc");
	
	// validate the function call is valid
	if(!is_callable("api_".$func)) {
		header("HTTP/1.0 404 Not Found");
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
	
	$userid = $_SESSION['userid'];
	$uploadsql = "SELECT up.id, up.userid, up.filename, up.timestamp, up.nsfw, up.tmbo, up.type, u.username,
		(SELECT COUNT(*) FROM offensive_subscriptions WHERE userid = $userid AND fileid = up.id) as subscribed,
		ca.good as vote_good, ca.bad as vote_bad, ca.tmbo as vote_tmbo, ca.repost as vote_repost, ca.comments
	FROM offensive_uploads up, offensive_count_cache ca, users u
	WHERE ca.threadid = up.id AND u.userid = up.userid AND up.status = 'normal'";

/**
	Helper functions.
**/

	/*
	 * Checks an incoming argument (usually in $_REQUEST) for type and validity.
	 * Arguments can be optional, or enforced to be one of a set of possibilities.
	 * If an argument fails to check, an error message is added to the global
	 * $errors array, and false is returned.  A false return value can be returned
	 * if an argument is optional and not set (but no error will be registered).
	 * Note that all arguments returned by this function that are not type "string"
	 * (not including the false return value) are safe to use in a SQL query.
	 */
	function check_arg($key, $type, $method=null, $required=true, $set=null) {
		global $errors;

		// make sure a valid method is being used.
		if($method == null) $method = $_REQUEST;
		// if this is triggered, there is a code problem.  kaboom.
		if(!is_array($method)) {
			trigger_error("ASSERT: invalid method", E_USER_ERROR);
			exit;
		}
		
		// optional vs required variables
		if(!array_key_exists($key, $method)) {
			if($required) {
				$errors[] = "required parameter '$key' is not set (needs: $type).";
			}
			if($type == "limit") return "0,200";
			return false;
		}
		
		// currently we don't accept arrays, which you can submit through POST
		if(!is_string($method[$key])) {
			$errors[] = "parameter '$key' must be of type $type.";
		}
		// this might change someday.  but not today.
		
		// general type enforcement
		switch($type) {
			case "string":
				if(strlen($method[$key]) == 0 && $required) {
					$errors[] = "parameter '$key' cannot be zero-length.";
					return false;
				}
				break;
			case "integer":
				if(!is_numeric($method[$key]) || strpos($method[$key], ".") !== false) {
					$errors[] = "parameter '$key' must be of type $type.";
					return false;
				}					
				break;
			case "float":
				if(!is_numeric($method[$key])) {
					$errors[] = "parameter '$key' must be of type $type.";
					return false;
				}
				break;
			/*
			 * dates are not set matched (it doesn't make much sense to), so
			 * we return the properly-formatted date representation of the argument
			 * that can be used immediately in a query.
			 */
			case "date":
				// unix timestamps, stop here.
				if(is_numeric($method[$key]) && strpos($method[$key], ".") === false) {
					return date("Y-m-d H:i:s", (int)$method[$key]);
				}
				// other date formats that we recognize, stop here.
				if(strtotime($method[$key]) !== false) {
					return date("Y-m-d H:i:s", strtotime($method[$key]));
				}
				$errors[] = "parameter '$key' is not a recognizable date string.";				
				return;
			/*
			 * Limits are special types.  They are in the form %d or %d,%d, matching the MySQL syntax
			 * for limits on queries.  This function detects if the argument is in the correct form, 
			 * and returns a strictly-formatted %d,%d string back to the caller, after enforcing 
			 * syntax and a maximum limit.
			 */
			case "limit":
				// %d,%d or %d only.
				$limit = $offset = false;

				/*
				 * Either the limit is a standalone integer...
				 */
				if(is_numeric($method[$key]) && strpos($method[$key], ".") === false) {
					$limit = (int)$method[$key];
					$offset = 0;
				/*
				 * Or it is in the format %d,%d, MySQL style.
				 */
				} else if(strpos($method[$key], ",") !== false) {
					$arr = explode(",", $method[$key]);

					/* only accept if there are two elements in the explosion, 
					 * both of which are integers.
					 */
					if(count($arr) == 2 &&
					   is_numeric($arr[0]) && strpos($arr[0], ".") === false &&
					   is_numeric($arr[1]) && strpos($arr[1], ".") === false ) {
						list($offset, $limit) = $arr;
					}
				}

				// did we make it?  is there a limit and an offset?
				if($limit === false || $offset === false) {
					$errors[] = "parameter '$key' is not in the correct format.  ".
					            "Accepted formats: '%d' and '%d,%d'.";
					return false;
				}

				/* force-coerce in case there are chars that are non-numeric, 
				 * but acceptable for php soft-coercion.
				 */
				$limit = (int)$limit;
				$offset = (int)$offset;

				// currently we enforce a limit of 200 elements per request.  just in case.
				if($limit > 200) {
					$errors[] = "the limit of parameter '$key' cannot exceed 200.  you requested: $limit.";
					return false;
				}

				return "$offset,$limit";
		}
		
		// check if a value is only allowed to be one in a limited set
		if(is_array($set)) {
			foreach($set as $example) {
				if($example == $method[$key]) {
					return $method[$key];
				}
			}
			$err = "parameter '$key' must be one of: { ";
			foreach($set as $example) {
				$err .= "$example ";
			}
			$errors[] = $err."}";
			return false;
		}
		
		// coerce values to the appropriate php type on return.
		switch($type) {
			case "float":
				return (double)$method[$key];
			case "integer":
				return (int)$method[$key];
			default:
				return $method[$key];
		}
	}

	/*
	 * Check the global $errors array for error messages.  If any exist, print them
	 * out and exit with an error code.
	 * This should always be called after the last argument is validated with
	 * check_arg to ensure that execution stops on error.
	 */
	function handle_errors() {
		global $errors;
		if(count($errors) == 0) return true;
		
		header("400 Bad Request");
		
		foreach($errors as $error) {
			echo "$error\n";
		}
		
		exit;
	}

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
		} else if(strtotime($val) > 0 && $rtype == "plist") {
			$data[$key] = date('c', strtotime($val));
		} else if(is_numeric($val)) {
			if(strpos($val, ".") === false) {
				$data[$key] = (int)$val;
			} else {
				$data[$key] = (double)$val;
			}
		} else if($val == "true") {
			$data[$key] = true;
		} else if($val == "false") {
			$data[$key] = false;
		}
	}
	
	/*
	 * return php data to the caller in the format requested.
	 */
	function send($ret) {
		global $rtype;
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
		
		format_data($row);
		
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
	
/**
	API functions
**/	
	call_user_func("api_".$func);

	function api_getuploads() {
		global $uploadsql;
		
		$type = check_arg("type", "string", null, false, array("image", "topic", "avatar"));
		$userid = check_arg("userid", "integer", null, false);
		$after = check_arg("after", "date", null, false);
		$before = check_arg("before", "date", null, false);
		$max = check_arg("max", "integer", null, false);
		$since = check_arg("since", "integer", null, false);
		$sort = check_arg("sort", "string", null, false, array("date_desc", "date_asc", "votes_asc", "votes_desc"));
		$limit = check_arg("limit", "limit", null, false);
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
		
		send($rows);
	}
	
	function api_getupload() {
		global $uploadsql;
		
		$upload = check_arg("upload", "integer");
		handle_errors();
		
		$sql = $uploadsql;
		
		$sql .= " AND up.id = $upload LIMIT 1";
		
		$row = get_row($sql);
		
		if($row === false) send(false);
		
		format_uprow($row);
		
		send($row);
	}
	
	function api_getyearbook() {
		global $uploadsql;
		
		$userid = check_arg("userid", "integer", null, false);
		$limit = check_arg("limit", "limit", null, false);
		$sort = check_arg("sort", "string", null, false, array("date_desc", "date_asc", "uname_asc", "uname_desc"));
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
			
			send($rows);
		}
	}
	
	function api_getuser() {
		$userid = check_arg("userid", "integer", null, false);
		handle_errors();

		if(!$userid) {
			$userid = $_SESSION['userid'];
		}
		
		$sql = "SELECT (SELECT COUNT(*) FROM users WHERE referred_by = $userid) as posse, (SELECT COUNT(*) FROM offensive_uploads WHERE type = 'avatar' AND userid = $userid) as yearbook, userid, username, created, account_status, timestamp, referred_by";
		if($_SESSION['status'] == "admin") {
			$sql .= ", email, last_login_ip";
		}
		$sql .= " FROM users WHERE userid = $userid";

		send(get_row($sql));
	}
	
	function api_getposse() {
		global $rtype;
		
		check_arg("userid", "integer", null, false);
		handle_errors();
		
		if(isset($_REQUEST['userid'])) {
			$userid = $_REQUEST['userid'];
		} else {
			$userid = $_SESSION['userid'];
		}
		
		$sql = "SELECT (SELECT COUNT(*) FROM users u WHERE referred_by = users.userid) as posse, (SELECT COUNT(*) FROM offensive_uploads WHERE type = 'avatar' AND userid = users.userid) as yearbook, userid, username, created, account_status, timestamp, referred_by";
		if($_SESSION['status'] == "admin") {
			$sql .= ", email, last_login_ip";
		}
		$sql .= " FROM users WHERE referred_by = $userid";
		
		send(get_rows($sql));
	}
	
	function api_login() {
		check_arg("username", "string");
		check_arg("password", "string");
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
	
	function api_logout() {
		session_unset();
		send(true);
	}
	
	function api_getcomments() {
		$votefilter = check_arg("votefilter", "string", null, false);
		$userfilter = check_arg("userfilter", "integer", null, false);
		$after = check_arg("after", "date", null, false);
		$before = check_arg("before", "date", null, false);
		$idmin = check_arg("idmin", "integer", null, false);
		$idmax = check_arg("idmax", "integer", null, false);
		$id = check_arg("id", "integer", null, false);
		$threadmin = check_arg("threadmin", "integer", null, false);
		$threadmax = check_arg("threadmax", "integer", null, false);
		$thread = check_arg("thread", "integer", null, false);
		$sort = check_arg("sort", "string", null, false, array("date_desc", "date_asc"));
		$limit = check_arg("limit", "limit", null, false);
		handle_errors();
		
		$sql = "SELECT com.fileid, com.id, com.comment, com.vote, com.offensive, com.repost, com.timestamp,
					   u.userid, u.username
				FROM offensive_comments com, users u
				WHERE com.userid = u.userid";
		
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
		
		for($i = 0; $i < count($rows); $i++) {
			if($rows[$i]['vote'] === null) unset($rows[$i]['vote']);
			if($rows[$i]['offensive'] === null) unset($rows[$i]['offensive']);
			if($rows[$i]['repost'] === null) unset($rows[$i]['repost']);
		}
		
		send($rows);
	}
	
	function api_postcomment() {
		$fileid = check_arg("fileid", "integer");
		$comment = check_arg("comment", "string", $_POST, false);
		$vote = check_arg("vote", "string", $_POST, false, array("this is good", "this is bad"));
		$offensive = check_arg("offensive", "integer", $_POST, false, array("1", "0"));
		$repost = check_arg("repost", "integer", $_POST, false, array("1", "0"));
		handle_errors();
		
		send(false);
	}
	
	function api_postupload() {
		$type = check_arg("type", "string", null, true, array("avatar", "image"));
		// FIX THIS TO WORK RIGHT!
		check_arg("filename", "string", $_FILE);
		$filename = check_arg("filename", "string", $_POST, false);
		$comment = check_arg("comment", "string", $_POST);
		$nsfw = check_arg("nsfw", "integer", $_POST, false, array("0", "1"));
		$tmbo = check_arg("tmbo", "integer", $_POST, false, array("0", "1"));
		handle_errors();
		
		send(false);
	}
	
	function api_posttopic() {
		$title = check_arg("title", "string", $_POST);
		$comment = check_arg("comment", "string", $_POST, false);
		handle_errors();
		
		send(false);
	}
	
	function api_searchcomments() {
		$q = check_arg("q", "string");
		$limit = check_arg("limit", "limit", null, false);
		handle_errors();
		
		$sql = "SELECT com.fileid, com.id, com.comment, com.vote, com.offensive, com.repost, com.timestamp,
					   u.userid, u.username
				FROM offensive_comments com, users u
				WHERE com.userid = u.userid AND MATCH(com.comment) AGAINST('".sqlEscape($q)."' IN BOOLEAN MODE)
				ORDER BY com.id LIMIT $limit";
		
		$rows = get_rows($sql);
		
		send($rows);
	}
	
	function api_searchuser() {
		$q = check_arg("q", "string");
		handle_errors();
		$q = sqlEscape($q);
		
		$sql = "SELECT userid FROM users WHERE username = '$q' LIMIT 1";
		$row = get_row($sql);
		if($row === false) send(false);

		$_REQUEST['userid'] = (string)$row['userid'];
		api_getuser();
	}
	
	function api_searchuploads() {
		global $uploadsql;
		$q = check_arg("q", "string");
		$limit = check_arg("limit", "limit", null, false);
		$type = check_arg("type", "string", null, false, array("image", "topic", "avatar"));
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
		
		send($rows);
	}
	
	function api_invite() {
		$email = check_arg("email", "string");
		handle_errors();
		
		send(false);
	}
	
	function api_faq() {
		send("<ul><li>Don't be retarded.</li></ul>");
	}
	
	function api_getlocation() {
		$userid = check_arg("userid", "integer", null, false);
		$minlat = check_arg("minlat", "float", null, false);
		$maxlat = check_arg("maxlat", "float", null, false);
		$minlong = check_arg("minlong", "float", null, false);
		$maxlong = check_arg("maxlong", "float", null, false);
		$limit = check_arg("limit", "limit", null, false);
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
		
		send($rows);
	}

	function api_setlocation() {
		$lat = check_arg("lat", "float");
		$long = check_arg("long", "float");
		$userid = $_SESSION['userid'];
		handle_errors();
		
		$sql = "REPLACE INTO maxxer_locations (userid, x, y, mapversion) VALUES( $userid, $lat, $long, 'google' )";
		$result = tmbo_query( $sql );
		send(true);
	}
	
	function api_pickupid() {
		send(false);
	}
	
	
	function api_unreadcomments() {
		global $uploadsql;
		
		$sort = check_arg("sort", "string", null, false, array("comment_desc", "comment_asc", "file_asc", "file_desc"));
		$limit = check_arg("limit", "limit", null, false);
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
		
		send($rows);
	}
	
	
	function api_subscribe() {
		$threadid = check_arg("threadid", "integer");
		$subscribe = check_arg("subscribe", "integer", null, false, array("1", "0"));
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

?>