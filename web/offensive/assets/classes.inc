<?

class Upload {
	private $data = array();
	
	public function __construct($info) {
		/*
		 * info must be an id or a database row array
		 * to include user information, the database row can include
		 * user_* columns that correspond to rows in the users table.
		 * if the argument array contains any user_* elements, it MUST
		 * include a user_userid element.
		 */
		if(is_array($info)) {
			if(!array_key_exists("id", $info))
				trigger_error("cannot prepopulate an Upload object without an id", E_USER_ERROR);
			/*
			 * prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 * that might change if there are issues with location, yearbook, or posse
			 */
			
			/*
			 * strip out keys that start with user_ into their own array,
			 * to be used to prepopulate a User object to go with this Upload object
			 */
			$userarr = array();
			foreach($info as $key => $val) {
				if(strpos($key, "user_") === 0) {
					$userarr[substr($key, 5)] = $val;
				} else {
					$this->data[$key] = $val;
				}
			}
			
			/*
			 * rescue the userid from the data as well, this will help
			 * avoid the failure case where user data not including the userid
			 * is included in the incoming data.
			 */
			if(array_key_exists("userid", $this->data))
				$userarr["userid"] = $this->data["userid"];
				
			// if we found any user data, create a User object from it.
			if(count($userarr) > 0) {
				$this->data["uploader"] = new User($userarr);
			}
		} else if(is_numeric($info) && strpos($info, ".") === false) {
			$this->data = array("id" => $info);
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	private function getUploadRow($element=false) {
		// try to close requests immediately if they are simple element queries
		if($element && array_key_exists($element, $this->data))
			return $this->data[$element];
		
		// get information we need from the database.
		$sql = "SELECT * FROM offensive_uploads WHERE id = ".$this->data["id"];
		$result = tmbo_query($sql);
		
		// if the upload doesn't exist...  well what can we do?
		if(mysql_num_rows($result) != 1) return false;
		
		$row = mysql_fetch_assoc($result);
		foreach($row as $key => $val) {
			$this->data[$key] = $val;
		}
		
		// now we should have enough information for element queries
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_uploads table, but was not available",
			              E_USER_WARNING);
		}
		return true;
	}
	
	public function exists() {
		return $this->getUploadRow();
	}
	
	/*
	 * functions that deal with data available in the database upload row
	 */
	
	public function id() {
		return $this->data["id"];
	}
	
	public function filename() {
		return $this->getUploadRow("filename");
	}
	
	public function timestamp() {
		return $this->getUploadRow("timestamp");
	}
	
	public function is_tmbo() {
		return ($this->getUploadRow("tmbo") == 1 ||
		        strpos(strtolower($this->filename()), "[tmbo]") !== false);
	}
	
	public function is_nsfw() {
		return ($this->getUploadRow("nsfw") == 1 ||
		        strpos(strtolower($this->filename()), "[nsfw]") !== false);
	}
	
	public function type() {
		return $this->getUploadRow("type");
	}
	
	// the status column is DEPRECATED and WILL be removed at a later point.
	public function status() {
		return $this->getUploadRow("status");
	}
	
	/*
	 * functions that deal with comments and votes
	 */
	private function getCacheRow($element=false) {
		// try to close requests immediately if they are simple element queries
		if($element && array_key_exists($element, $this->data))
			return $this->data[$element];
        
		// get information we need from the database.
		$sql = "SELECT good AS goods, bad AS bads, tmbo AS tmbos, repost AS tiars, comments,
		        timestamp AS last_active
		        FROM offensive_count_cache WHERE threadid = ".$this->data["id"];
		$result = tmbo_query($sql);
        
		// if the upload doesn't exist...  well what can we do?
		if(mysql_num_rows($result) != 1) return false;
        
		$row = mysql_fetch_assoc($result);
		foreach($row as $key => $val) {
			$this->data[$key] = $val;
		}
        
		// now we should have enough information for element queries
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_count_cache table, but was not available",
			              E_USER_WARNING);
		}
		return true;
	}
	
	public function goods() {
		return $this->getCacheRow("goods");
	}
	
	public function bads() {
		return $this->getCacheRow("bads");
	}
	
	public function tmbos() {
		return $this->getCacheRow("tmbos");
	}
	
	public function tiars() {
		return $this->getCacheRow("tiars");
	}
	
	public function comments() {
		return $this->getCacheRow("comments");
	}
	
	public function last_active() {
		return $this->getCacheRow("last_active");
	}
	
	/*
	 * User slinging code
	 */
	
	public function uploader() {
		if(!array_key_exists("uploader", $this->data)) {
			if(array_key_exists("userid", $this->data)) {
				/*
				 * if we already have a userid, the User class can do the
				 * database work, it already knows how.
				 */
				$this->data["uploader"] = new User($this->data["userid"]);
			} else {
				/*
				 * if we have to go to the database anyway, we may as well
				 * grab the entire user row, to prevent a duplicate database call
				 * from the User class
				 */
				$sql = "SELECT u.* FROM users u, offensive_uploads up 
				        WHERE u.userid = up.userid AND up.id = ".$this->data["id"];
				
				$result = tmbo_query($sql);
				if(mysql_num_rows($result) !== 1)
					trigger_error("user failure, user for this upload does not exist?", E_USER_ERROR);
				$this->data["uploader"] = new User(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["uploader"];
	}
	
	/*
	 * comment slinging code
	 */
	public function getComments() {
		if(!array_key_exists("comments", $this->data)) {
			$this->data["comments"] = array();
			
			$sql = "SELECT com.*, u.userid AS user_userid, u.username AS user_username,
			        u.account_status AS user_account_status
			        FROM offensive_comments com, users u
			        WHERE u.userid = com.userid AND fileid = ".$this->data["id"].
			       " ORDER BY timestamp ASC";
			$result = tmbo_query($sql);
			while(false !== ($row = mysql_fetch_assoc($result))) {
				$this->data["comments"][] = new Comment($row);
			}
		}
		
		return $this->data["comments"];
	}
	
	/*
	 * upload slinging code
	 */
	public function next() {
		if(!array_key_exists("next", $this->data)) {
			$sql = "SELECT MIN( id ) AS next FROM offensive_uploads 
		        	WHERE id > ".$this->id()." AND type='".$this->type()."' AND status='normal'";
			$result = tmbo_query($sql);
			$row = mysql_fetch_assoc($result);
			if($row["next"] == null)
				$this->data["next"] = false;
			else
				$this->data["next"] = new Upload($row["next"]);
		}
		
		return $this->data["next"];
	}
	
	public function prev() {
		if(!array_key_exists("prev", $this->data)) {
			$sql = "SELECT MAX( id ) AS prev FROM offensive_uploads
		        	WHERE id < ".$this->id()." AND type='".$this->type()."' AND status='normal'";
			$result = tmbo_query($sql);
			$row = mysql_fetch_assoc($result);
			if($row["prev"] == null)
				$this->data["prev"] = false;
			else
				$this->data["prev"] = new Upload($row["prev"]);
		}
		
		return $this->data["prev"];
	}
	
	/*
	 * string slinging code
	 */
	
	public function file() {
		// XXX: eventually getFile should reside inside this function, not callable from outside.
		return getFile($this->id(), $this->filename(), $this->timestamp(), $this->type());
	}
	
	public function URL() {
		// XXX: eventually getFileURL should reside inside this function, not callable from outside.
		return getFileURL($this->id(), $this->filename(), $this->timestamp(), $this->type());
	}
	
	public function thumb() {
		// XXX: eventually getThumb should reside inside this function, not callable from outside.
		return getThumb($this->id(), $this->filename(), $this->timestamp(), $this->type());
	}
	
	public function thumbURL() {
		// XXX: eventually getThumbURL should reside inside this function, not callable from outside.
		return getThumbURL($this->id(), $this->filename(), $this->timestamp(), $this->type());
	}
	
	public function commentLabel($showvotes=true) {
		// XXX: eventually commentLabel should reside inside this function, not callable from outside.
		return commentLabel($this->comments(), $this->goods(), $this->bads(), $this->tmbos(), $showvotes);
	}

	public function htmlFilename($showtype=false) {
		$arg = array(
			"filename" => $this->filename(),
			"nsfw" => $this->is_nsfw(),
			"tmbo" => $this->is_tmbo(),
			"type" => $this->type()
			);
		return htmlFilename($arg, $showtype);
	}
}

class Comment {
	private $data = array();
	
	public function __construct($info) {
		/*
		 * info must be an id or a database row array
		 * to include user information, the database row can include
		 * user_* columns that correspond to rows in the users table.
		 * if the argument array contains any user_* elements, it MUST
		 * include a user_userid element.
		 */
		if(is_array($info)) {
			if(!array_key_exists("id", $info))
				trigger_error("cannot prepopulate a Comment object without an id", E_USER_ERROR);
			/*
			 * prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 * that might change if there are issues with location, yearbook, or posse
			 */
			
			/*
			 * strip out keys that start with user_ into their own array,
			 * to be used to prepopulate a User object to go with this Upload object
			 */
			$userarr = array();
			foreach($info as $key => $val) {
				if(strpos($key, "user_") === 0) {
					$userarr[substr($key, 5)] = $val;
				} else {
					$this->data[$key] = $val;
				}
			}
			
			/*
			 * rescue the userid from the data as well, this will help
			 * avoid the failure case where user data not including the userid
			 * is included in the incoming data.
			 */
			if(array_key_exists("userid", $this->data))
				$userarr["userid"] = $this->data["userid"];
				
			// if we found any user data, create a User object from it.
			if(count($userarr) > 0) {
				$this->data["commenter"] = new User($userarr);
			}
		} else if(is_numeric($info) && strpos($info, ".") === false) {
			$this->data = array("id" => $info);
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	private function getCommentRow($element=false) {
		// try to close requests immediately if they are simple element queries
		if($element && array_key_exists($element, $this->data))
			return $this->data[$element];
		
		// get information we need from the database.
		$sql = "SELECT * FROM offensive_comments WHERE id = ".$this->data["id"];
		$result = tmbo_query($sql);
		
		// if the comment doesn't exist...  well what can we do?
		if(mysql_num_rows($result) != 1) return false;
		
		$row = mysql_fetch_assoc($result);
		foreach($row as $key => $val) {
			$this->data[$key] = $val;
		}
		
		// now we should have enough information for element queries
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_comments table, but was not available",
			              E_USER_WARNING);
		}
		return true;
	}
	
	public function id() {
		return $this->data["id"];
	}
	
	public function exists() {
		return $this->getCommentRow();
	}
	
	public function vote() {
		return $this->getCommentRow("vote");
	}
	
	public function tmbo() {
		// XXX: this should eventually return a boolean
		return $this->getCommentRow("offensive");
	}
	
	public function tiar() {
		// XXX: this should eventually return a boolean
		return $this->getCommentRow("repost");
	}
	
	public function timestamp() {
		return $this->getCommentRow("timestamp");
	}
	
	public function text() {
		return $this->getCommentRow("comment");
	}
	
	public function HTMLcomment() {
		$comment = $this->comment();
		if($comment == "") return "";
		
		// format and maybe support some amount of bbcode?
		// booyeah.
	}
	
	public function upload() {
		return new Upload($this->getCommentRow("fileid"));
	}

	/*
	 * User slinging code
	 */
	public function commenter() {
		if(!array_key_exists("commenter", $this->data)) {
			if(array_key_exists("userid", $this->data)) {
				/*
				 * if we already have a userid, the User class can do the
				 * database work, it already knows how.
				 */
				$this->data["commenter"] = new User($this->data["userid"]);
			} else {
				/*
				 * if we have to go to the database anyway, we may as well
				 * grab the entire user row, to prevent a duplicate database call
				 * from the User class
				 */
				$sql = "SELECT u.* FROM users u, offensive_uploads up 
				        WHERE u.userid = up.userid AND up.id = ".$this->data["id"];
				
				$result = tmbo_query($sql);
				if(mysql_num_rows($result) !== 1)
					trigger_error("user failure, user for this upload does not exist?", E_USER_ERROR);
				$this->data["commenter"] = new User(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["commenter"];
	}
	
}

class User {
	private $data = array();
	
	public function __construct($info) {
		if(is_array($info)) {
			if(!array_key_exists("userid", $info))
				trigger_error("cannot prepopulate a User object without a userid", E_USER_ERROR);
			/*
			 * prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 * that might change if there are issues with location, yearbook, or posse
			 */
			$this->data = $info;
		} else if(is_numeric($info) && strpos($info, ".") === false) {
			$this->data = array("userid" => $info);
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	/* populate the internal data structure with information from the row
	 * in the users table belonging to this user.
	 */
	private function getUserRow($element=false) {
		// try to close requests immediately if they are simple element queries
		if($element && array_key_exists($element, $this->data))
			return $this->data[$element];
		
		// get information we need from the database.
		$sql = "SELECT * FROM users WHERE userid = ".$this->data["userid"];
		$result = tmbo_query($sql);
		
		// if the user doesn't exist...  well what can we do?
		if(mysql_num_rows($result) != 1) return false;
		
		$row = mysql_fetch_assoc($result);
		foreach($row as $key => $val) {
			$this->data[$key] = $val;
		}
		
		// now we should have enough information for element queries
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from user table, but was not available", E_USER_WARNING);
		}
		return true;
	}
	
	public function exists() {
		return $this->getUserRow();
	}
	
	/*
	 * These functions handle information that's usually available through the
	 * users table of the database.  they call getUserRow because it's not ever
	 * guaranteed that the data requested will be immediately available — it may
	 * require a round-trip to the database to get the information requested.
	 */
	public function id() {
		return $this->data["userid"];
	}
	
	public function username() {
		return $this->getUserRow("username");
	}
	
	public function email() {
		return $this->getUserRow("email");
	}
	
	public function created() {
		return $this->getUserRow("created");
	}
	
	public function status() {
		return $this->getUserRow("account_status");
	}
	
	public function timestamp() {
		return $this->getUserRow("timestamp");
	}
	
	public function referred_by() {
		if($this->getUserRow("referred_by") == $this->data["userid"]) {
			return false;
		}
		return new User($this->getUserRow("referred_by"));
	}
	
	/*
	 * advanced user row stuff
	 */
	
	public function posse() {
		if(!array_key_exists("posse", $this->data)) {
			$sql = "SELECT * FROM users WHERE referred_by = ".$this->data["userid"]. "
			        AND userid != ".$this->data["userid"];
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) === 0)
				$this->data["posse"] = false;
			else {
				$this->data["posse"] = array();
				while(false !== ($row = mysql_fetch_assoc($result)))
					$this->data["posse"][] = new User($row);
			}
		}
		
		return $this->data["posse"];
	}
	
	/*
	 * location data
	 */
	
	public function location() {
		if(!array_key_exists("location", $this->data)) {
		
			$sql = "SELECT x AS latitude, y AS longitude, timestamp 
			        FROM maxxer_locations WHERE userid = ".$this->data["userid"];
			$result = tmbo_query($sql);
			
			if(mysql_num_rows($result) !== 1)
				$this->data["location"] = false;
			else
				$this->data["location"] = mysql_fetch_assoc($result);
		}
		
		return $this->data["location"];
	}
	
	/*
	 * yearbook data
	 */
	
	public function yearbook() {
		if(!array_key_exists("yearbook", $this->data)) {
			$sql = "SELECT id FROM offensive_uploads WHERE type = 'avatar' AND userid = ".$this->data["userid"].
			       " ORDER BY timestamp DESC LIMIT 1";
			$result = tmbo_query($sql);
			
			if(mysql_num_rows($result) !== 1)
				$this->data["yearbook"] = false;
			else {
				$row = mysql_fetch_assoc($result);
				$this->data["yearbook"] = new Upload($row["id"]);
			}
		}
		
		return $this->data["yearbook"];
	}
}

?>