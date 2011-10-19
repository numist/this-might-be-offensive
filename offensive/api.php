<?
	set_include_path("..");
	// set up the normal TMBO environment
	require_once( 'offensive/assets/header.inc' );
	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/functions.inc");
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	require_once("offensive/assets/comments.inc");
		
	class Error {
		private $msg;

		function __construct($errmsg) {
			$this->msg = $errmsg;
		}

		public function api_data() {
			return $this->msg;
		}
	}

	function called_by_api() {
		return true;
	}
	
	require_once("offensive/assets/argvalidation.inc");
		
	// get function name and return type
	$broken = explode("/", $_SERVER["REQUEST_URI"]);
	$call = array_pop($broken);
	list($func, $rtype) = explode(".", $call);
	if(strpos($rtype, "?") !== false) {
		$broken = explode("?", $rtype);
		$rtype = array_shift($broken);
	}
	
	// return type validation and definitions:
	require_once("offensive/assets/output/json.inc");
	require_once("offensive/assets/output/php.inc");
	require_once("offensive/assets/output/plist.inc");
	require_once("offensive/assets/output/xml.inc");
	$rtype = strtolower($rtype);
	if(!is_callable("tmbo_".$rtype."_encode")) {
		header("HTTP/1.0 400 Bad Request");
		header("Content-type: text/plain");
		echo "unsupported return format $rtype.";
		exit;
	}
	
	// validate the function call is valid
	if(!is_callable("api_".$func)) {
		header("HTTP/1.0 404 Not Found");
		header("Content-type: text/plain");
		send(new Error("the function you requested ($func) was not found on this server."));
	}
	
	// authentication
	if($func != "login") {
		mustLogIn(array("prompt" => "http",
		                "token" => null));
	}
	
/**
	Helper functions.
*/

	/*
	 * coerce values of an array from strings to the appropriate PHP types.
	 */
	function format_data(&$data) {
		global $rtype;
		
		if(is_object($data)) return;
		
		if(is_array($data)) {
			foreach($data as $key => $val) {
				format_data($data[$key]);
			}
		/* plists get a special date format as their output,
		 * due to spec restrictions.
		 */
		} else if(strtotime($data) > 0 && !is_numeric($data) && $rtype == "plist") {
			$data = gmdate('c', strtotime($data));
		} else if(is_numeric($data)) {
			if(is_intger($data)) {
				$data = (int)$data;
			} else {
				$data = (double)$data;
			}
		} else if($data === "true") {
			$data = true;
		} else if($data === "false") {
			$data = false;
		}
		if(is_string($data) && $rtype == "xml") {
			$data = str_replace(array("&", "<", ">"), array("&amp;", "&lt;", "&gt;"), $data);
		}
	}
	
	/*
	 * return php data to the caller in the format requested.
	 */
	require_once("offensive/assets/conditionalGet.inc");
	function send($ret) {
		global $rtype;
		
		// get the newest timestamp in the dataset (if possible) and conditional GET
		if(is_object($ret) && method_exists($ret, "timestamp")) {
				conditionalGet($ret->timestamp());
		} else if(is_array($ret) && count($ret) > 0) {
			$timestamp = 0;
			foreach($ret as $val) {
				if(is_object($val) && method_exists($val, "timestamp") && 
				   // this line is silly, but it avoids an extra call to strtotime.
				   ($tmpstamp = strtotime($val->timestamp())) > $timestamp) {
					$timestamp = $tmpstamp;
				}
			}
			if($timestamp > 0) {
				conditionalGet($timestamp);
			}
		}
		
		// send the data back
		if(in_array($rtype, array("plist", "xml"))) {
			header("Content-type: text/xml");
		} else {
			header("Content-type: text/plain");
		}
		echo call_user_func("tmbo_".$rtype."_encode", $ret);
		exit;
	}
	
	/* XXX upload rows returned by API should include:
	 * subscribed, nsfw, tmbo, filename, file link, file dims (if image),
	 * thumb link (if image), thumb dims (if image), id, type, next?!, prev?! (of same type).
	 */	
	require_once("offensive/assets/comments.inc");

/**
	API functions
*/	
	call_user_func("api_".$func);

	/**
	 * @method getuploads
	 * Get one or more uploads' metadata, including (if possible) links to the uploaded files themselves, and their thumbnails.
	 * @param type string optional {"image", "topic", "avatar", "audio"} Get uploads of this type.
	 * @param userid integer optional Get uploads posted by this user.
	 * @param after date optional Get uploads newer than this.
	 * @param before date optional Get uploads older than this.
	 * @param max integer optional Get uploads before this fileid, inclusive.
	 * @param since integer optional Get uploads since this fileid, inclusive.
	 * @param sort string optional {"date_desc", "date_asc", "votes_asc", "votes_desc", "comments_asc", "comments_desc", "activity_asc", "activity_desc"} Default:"date_desc" Result sorting. Sort by vote does not consider [-].
	 * @param limit limit optional Default and maximum is 200.
	 * @param nsfw integer optional {"0", "1"} Set to 0, suppresses posts that are not safe for work. Set to 1, only returns results that are not safe for work.
	 * @param tmbo integer optional {"0", "1"} Set to 0, suppresses posts that might be offensive. Set to 1, only returns results that might be offensive.
	 * @return Array of Upload objects.
	 * @example type=image&nsfw=0&sort=date_asc&limit=3,10
	 * @see getupload
	 */
	function api_getuploads() {
		send(core_getuploads($_REQUEST));
	}
	
	function api_getchanges() {
		global $redislink;
		$since = check_arg("since", "integer");
		handle_errors();
		$changes = $redislink->lrange("changelog", 0, 100);
		$ret_changes = array();
		foreach ($changes as $change) {
			$entry = explode(":", $change, 3);
			if ($since < $entry[1]) {
				$ret_changes[$entry[1]] = $entry[2];
			} else {
				break;
			}
		}
		send($ret_changes);
	}

	/**
	 * @method getupload
	 * Get an upload's metadata, including (if possible) links to the uploaded files themselves, and their thumbnails.
	 *
	 * Calling this function updates your pickuplink.
	 *
	 * @param fileid integer required Get upload with this id.
	 * @return A singleton Upload object or an Error object if upload does not exist.
	 * @example fileid=2
	 * @example fileid=112
	 * @see getuploads
	 */
	function api_getupload() {
		global $uploadsql;
		
		$arg = check_arg("fileid", "integer");
		handle_errors();
		
		$upload = new Upload($arg);
		
		if(!$upload->exists()) {
			send(new Error("upload $arg does not exist"));
		}
		
		send($upload);
	}

	/**
	 * @method getyearbook
	 * Get upload metadata for one or more users' current avatars.
	 *
	 * To access non-current avatars, use getuploads(type=avatar).
	 *
	 * @param userid integer optional Get the current avatar for this user only.
	 * @param limit limit optional Default and maximum is 200.
	 * @param sort string optional {"date_desc", "date_asc", "uname_asc","uname_desc"} Default:"uname_desc" Sort by username or upload date.
	 * @return A singleton Upload object if userid is specified, an array of Upload objects otherwise. Returns false if the userid does not exist.
	 * @example userid=2054
	 * @example limit=2&sort=date_asc
	 * @see getuploads
	 */
	function api_getyearbook() {
		send(core_getyearbook($_REQUEST));
	}

	/**
	 * @method getuser
	 * Get a user's metadata.
	 *
	 * @param userid integer optional Get metadata for this user. Defaults to the current user.
	 * @return If the userid exists, returns a user metadata object. Returns false if the userid does not exist.
	 * @example userid=151
	 */
	function api_getuser() {
		$userid = check_arg("userid", "integer", null, false);
		handle_errors();

		if(!$userid) {
			assert('me()');
			$userid = me()->id();
		}
		
		$ret = new User($userid);
		
		if($ret->exists()) {
			send($ret);
		} else {
			send(new Error("user $userid does not exist"));
		}
	}

	/**
	 * @method getposse
	 * Get a user's posse.
	 *
	 * @param userid integer optional Get metadata for this user. Defaults to the current user.
	 * @return If the userid exists, returns a possibly-empty array of user metadata objects. Returns false if userid does not exist.
	 * @example userid=2054
	 * @example userid=22
	 */
	function api_getposse() {
		$userid = check_arg("userid", "integer", null, false);
		handle_errors();
		
		if($userid === false) {
			assert('me()');
			$userid = me()->id();
		}
		
		$user = new User($userid);
		
		$posse = $user->posse();
		
		send($posse);
	}

	/**
	 * @method login
	 * Establish a session for future requests and/or generate a token.
	 *
	 * The token is named after the user-agent of the requesting application and can be revoked by the user at any time in /offensive/?c=settings.
	 *
	 * @param username string required User's name.
	 * @param password string required User's password.
	 * @param gettoken integer optional {"0", "1"} Do you want to generate a new token?
	 * @return a token object or the metadata for the current user.
	 * @example username=jonxp&password=tester
	 * @example username=ray&password=tester&gettoken=1
	 * @see getuser
	 * @see logout
	 */
	function api_login() {
		$username = check_arg("username", "string");
		$password = check_arg("password", "string");
		$token = check_arg("gettoken", "integer", null, false, array("0", "1"));
		handle_errors();
		session_unset();
		
		$loggedin = login(array("u/p" => array($username, $password)));
		if($loggedin === false) {
			global $login_message;
			header("HTTP/1.0 401 Unauthorized");
			send(new Error($login_message));
			exit;
		} else if($loggedin === null) {
			global $login_message;
			header("HTTP/1.0 403 Forbidden");
			send(new Error($login_message));
			exit;
		}

		// XXX: do not create a session if this is set!
		if($token) {
			$agent = array_key_exists("HTTP_USER_AGENT", $_SERVER) ?
			         trim($_SERVER['HTTP_USER_AGENT']) :
			         " (no name)";
			send(core_createtoken($agent));
		} else {
			assert('me()');
			$_REQUEST['userid'] = me()->id();
			api_getuser();
		}
	}

	/**
	 * @method logout
	 * Destroys the current session and invalidates the rememberme cookie.
	 *
	 * Unnecessary if you are only using a token to authenticate, which doesn't create a login session.
	 *
	 * @return true
	 * @example
	 * @see login
	 */
	function api_logout() {
		session_unset();
		send(true);
	}

	/**
	 * @method getcomments
	 * 
	 *
	 * @param votefilter string optional Can contain any of "+-xrc" in any order. Other characters are ignored. Applies the following filters:
	 * •&nbsp;<b>+</b> - vote is 'this is good'.
	 * •&nbsp;<b>-</b> - vote is 'this is bad'.
	 * •&nbsp;<b>x</b> - vote includes 'this might be offensive'.
	 * •&nbsp;<b>r</b> - vote includes 'this is a repost'.
	 * •&nbsp;<b>c</b> - comment is not empty.
	 * @param userid integer optional Get comments made by this user only.
	 * @param after date optional Get comments newer than this date.
	 * @param before date optional Get comments older than this date.
	 * @param idmin integer optional Get comments after this commentid, inclusive.
	 * @param idmax integer optional Get comments before this commentid, inclusive.
	 * @param id integer optional Get comment with this id.
	 * @param threadmin integer optional Get comments from threads after this one, inclusive.
	 * @param threadmax integer optional Get comments from threads before this one, inclusive.
	 * @param thread integer optional Get comments from this thread. If you are subscribed to this thread, your subscription will be reset.
	 * @param sort string optional {"date_desc", "date_asc"} Default:"date_desc" Sort by comment date.
	 * @param limit limit optional Default and maximum is 200.
	 * @return An array of comment objects.
	 * @example thread=112
	 */
	function api_getcomments() {
		send(core_getcomments($_REQUEST));
	}

	function api_postcomment() {
		$fileid = check_arg("fileid", "integer", $_POST);
		$comment = check_arg("comment", "string", $_POST, false);
		$vote = check_arg("vote", "string", $_POST, false, array("this is good", "this is bad", "novote"));
		$offensive = check_arg("offensive", "integer", $_POST, false, array("1", "0"));
		$repost = check_arg("repost", "integer", $_POST, false, array("1", "0"));
		$subscribe = check_arg("subscribe", "integer", $_POST, false, array("1", "0"));
		handle_errors();
		assert('me()');
		
		// if no comment, vote, offensive, or repost, then why are you here?
		if(!($comment || $vote || $offensive || $repost || $subscribe)) {
			trigger_error("no comment, vote, tmbo, or tiar set -- nothing to do!", E_USER_WARNING);
			send(false);
		}
		
		if($vote == "novote") $vote = "";
		if($comment === false) $comment = "";
		if($vote === false) $vote = "";
		if($offensive === false) $offensive = 0;
		if($repost === false) $repost = 0;

		if($comment || $vote || $offensive || $repost) {
			postComment($fileid, $vote, $repost, $offensive, $comment);
		}
		
		if($subscribe == 1) {
			subscribe($fileid);
		}
		send(true);
	}

	// XXX: unimplemented
	function api_postupload() {
		$type = check_arg("type", "string", null, true, array("avatar", "image"));
		// XXX: FIX THIS TO WORK RIGHT!
		check_arg("filename", "string", $_FILE);
		$filename = check_arg("filename", "string", $_POST, false);
		$comment = check_arg("comment", "string", $_POST);
		$nsfw = check_arg("nsfw", "integer", $_POST, false, array("0", "1"));
		$tmbo = check_arg("tmbo", "integer", $_POST, false, array("0", "1"));
		handle_errors();
		
		// XXX: this will have to call some uploader helper functions to cooperate with the upload page.
		
		trigger_error("unimplemented", E_USER_ERROR);
	}

	// XXX: unimplemented
	function api_posttopic() {
		$title = check_arg("title", "string", $_POST);
		$comment = check_arg("comment", "string", $_POST, false);
		handle_errors();
		
		trigger_error("unimplemented", E_USER_ERROR);
	}

	function api_searchcomments() {
		send(core_searchcomments($_REQUEST));
	}

	/**
	 * @method searchuser
	 *
	 * Find a user by username.
	 *
	 * @param q string true Full or partial username.
	 * @param limit limit optional Default and maximum is 200.
	 * @return Array of User objects matching query string.
	 * @example q=max&limit=3
	 */
	function api_searchuser() {
		send(core_searchuser($_REQUEST));
	}

	/**
	 * @method searchuploads
	 *
	 * Find an upload by filename.
	 *
	 * @param q string true Full or partial filename.
	 * @param limit limit optional Default and maximum is 200.
	 * @param type string optional {"image", "topic", "avatar", "audio"} Find only uploads of this type.
	 * @return Array of Upload objects matching query string.
	 * @example q=weekend&limit=5
	 * @see login
	 */
	function api_searchuploads() {
		send(core_searchuploads($_REQUEST));
	}

	function api_invite() {
		$email = check_arg("email", "string", $_REQUEST);
		handle_errors();
		
		trigger_error("unimplemented", E_USER_ERROR);
	}

	/**
	 * @method getlocation
	 *
	 * Get the reported location of one or more maxxers.
	 *
	 * This data is user-reported, so it's likely that everyone is lying.
	 *
	 * The format of the data returned from this call is unstable and subject to change. Do not use this in a shipping application.
	 *
	 * @param userid integer optional Get the current location for this user only.
	 * @param minlat float optional Get locations with latitudes larger than this.
	 * @param maxlat float optional Get locations with latitudes smaller than this.
	 * @param minlong float optional Get locations with longitudes larger than this.
	 * @param maxlat float optional Get locations with longitudes larger than this.
	 * @param limit limit optional Default and maximum is 200.
	 * @return An array of location data from the database matching the parameters.
	 * @example limit=5
	 * @example userid=2054
	 * @see setlocation
	 */
	function api_getlocation() {
		$userid =  check_arg("userid",  "integer", null, false);
		$minlat =  check_arg("minlat",  "float",   null, false);
		$maxlat =  check_arg("maxlat",  "float",   null, false);
		$minlong = check_arg("minlong", "float",   null, false);
		$maxlong = check_arg("maxlong", "float",   null, false);
		$limit =   check_arg("limit",   "limit",   null, false);
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
		
		$result = tmbo_query($sql);
		
		$rows = array();
		while(($rows[] = mysql_fetch_assoc($result)) || array_pop($rows));
		send($rows);
	}

	/**
	 * @method setlocation
	 * 
	 * Set a user's location
	 *
	 * @param lat float required User's latitude in degrees.
	 * @param long float required User's longitude in degrees.
	 * @return true
	 * @example lat=50&long=50
	 * @see getlocation
	 */
	function api_setlocation() {
		$lat = check_arg("lat", "float");
		$long = check_arg("long", "float");
		assert('me()');
		$userid = me()->id();
		handle_errors();
		
		$sql = "REPLACE INTO maxxer_locations (userid, x, y, mapversion) VALUES( $userid, $lat, $long, 'google' )";
		$result = tmbo_query( $sql );
		send(true);
	}

	// XXX: unimplemented (also, topic pickup and audio pickup forthcoming)
	function api_pickup_image() {
		trigger_error("unimplemented", E_USER_ERROR);
	}
	
	function  api_pickup_topic() {
		trigger_error("unimplemented", E_USER_ERROR);
	}

	/**
	 * @method unreadcomments
	 *
	 * Get a list of unchecked activity on subscribed threads.
	 *
	 * @param sort string optional {"comment_desc", "comment_asc", "file_asc", "file_desc"} Default:"file_asc" Sort order of results.
	 * @param limit limit optional Default and maximum is 200.
	 * @return Array of Comment objects containing the oldest unread comment for each thread with unread comments.
	 * @example limit=3&sort=file_asc
	 * @see getcomments
	 */
	function api_unreadcomments() {
		send(core_unreadcomments($_REQUEST));
	}
	
	/**
	 * @method subscribe
	 *
	 * (Un)Subscribe to new posts in a thread.
	 *
	 * @param threadid integer required Thread you're adding/removing from your subscriptions.
	 * @param subscribe integer required {"1", "0"} 0 -> unsubscribe, 1 -> subscribe.
	 * @return login
	 * @example threadid=211604&subscribe=1
	 * @see login
	 */
	function api_subscribe() {	
		$threadid = check_arg("threadid", "integer");
		$subscribe = check_arg("subscribe", "integer", null, array("1", "0"));
		handle_errors();
		
		if($subscribe == 0) {
			send(unsubscribe($threadid));
		}
		send(subscribe($threadid));
	}

?>