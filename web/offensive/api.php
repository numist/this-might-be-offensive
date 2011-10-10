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
	
	// if not logged in, force a switch to ssl.
	if(!loggedin() && (!isset($_SERVER["HTTPS"]) || $_SERVER["HTTPS"] != "on")) {
		header("Location: https://".$_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"], 301);
		exit;
	}
	
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
	
	$me = false;
	// authentication
	if($func != "login" && !loggedin(false)) {
		mustLogIn("http");
	} else if(loggedin()) {
		// XXX: update ip_history, last_seen
		$me = new User($_SESSION['userid']);
	}
	
/**
	Helper functions.
**/

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
**/	
	call_user_func("api_".$func);

	function api_getuploads() {
		send(core_getuploads($_REQUEST));
	}
	
	function api_getchanges() {
		global $redislink;
		$since = check_arg("since", "integer");
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

	function api_getupload() {
		global $uploadsql;
		
		$upload = check_arg("fileid", "integer");
		handle_errors();
		
		$upload = new Upload($upload);
		
		if(!$upload->exists()) {
			send(new Error("upload $upload does not exist"));
		}
		
		send($upload);
	}

	function api_getyearbook() {
		send(core_getyearbook($_REQUEST));
	}

	function api_getuser() {
		$userid = check_arg("userid", "integer", $args, false);
		handle_errors();

		if(!$userid && isset($_SESSION) && array_key_exists('userid', $_SESSION)) {
			$userid = $_SESSION['userid'];
		}
		
		$ret = new User($userid);
		
		if($ret->exists()) {
			send($ret);
		} else {
			send(new Error("user $userid does not exist"));
		}
	}

	function api_getposse() {
		$userid = check_arg("userid", "integer", null, false);
		handle_errors();
		
		if($userid === false) {
			$userid = $_SESSION['userid'];
		}
		
		$user = new User($userid);
		
		$posse = $user->posse();
		
		send($posse);
	}

	function api_login() {
		check_arg("username", "string", null);
		check_arg("password", "string", null);
		handle_errors();
		session_unset();
		
		$loggedin = login($_REQUEST['username'], $_REQUEST['password']);
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
		$_REQUEST['userid'] = $_SESSION['userid'];
		api_getuser();
	}

	function api_logout() {
		session_unset();
		send(true);
	}

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
		
		$me = new User($_SESSION['userid']);
		
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

	// NOTE: to upload a file from inside the codebase, do not call this function!
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

	function api_searchuser() {
		send(core_searchuser($_REQUEST));
	}

	function api_searchuploads() {
		send(core_searchuploads($_REQUEST));
	}

	function api_invite() {
		$email = check_arg("email", "string", $_REQUEST);
		handle_errors();
		
		trigger_error("unimplemented", E_USER_ERROR);
	}

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

	// XXX: unimplemented (also, topic pickup and audio pickup forthcoming)
	function api_pickup_image() {
		trigger_error("unimplemented", E_USER_ERROR);
	}
	
	function  api_pickup_topic() {
		trigger_error("unimplemented", E_USER_ERROR);
	}

	function api_unreadcomments() {
		send(core_unreadcomments($_REQUEST));
	}
	
	function api_subscribe() {	
		$threadid = check_arg("threadid", "integer");
		$subscribe = check_arg("subscribe", "integer", null, false, array("1", "0"));
		handle_errors();
		
		if($subscribe == 0) {
			send(unsubscribe($threadid));
		}
		send(subscribe($threadid));
	}

?>
