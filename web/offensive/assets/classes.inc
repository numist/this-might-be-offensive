<?

require_once("offensive/assets/functions.inc");

/*
The purpose in creating a bunch of classes to handle users, uploads, and comments is to try and
alleviate the number of queries existing in display code, and to give the code a logical way
to traverse related datas.

From a usability perspective, it's completely possible to take an upload and find a commenter's
yearbook image by traversing $upload->getComments()[0]->commenter()->yearbook(); aside from PHP's
inability to execute the getComments()[0] portion of the code, this will work perfectly well.

From a performance perspective, this code aggressively caches, and reduces the number of database
calls necessary to fetch data.  All public functions should not incur a database call after the
first call.  for example, $upload->exists() might go to the database the first time it is called,
but afterwards the result is cached, so all future calls to $upload->exists() will complete
immediately.  As a bonus, whenever a function goes to the database, it uses a generic query, which
gives us a few additional benefits.  First, this query returns a complete row, so the cache
contains additional information (in the case of $upload->exists(), you would also now have
filename, is_tmbo, is_nsfw, timestamp, etc cached).  Second, this increases the likelihood of
MySQL experiencing a database cache hit, which also speeds things up considerably.

In addition, the data store for these classes is centralized, so an object made in one function
that gets data for Fipi Lele will load data from the database, and then future User objects for
Fipi Lele will also take advantage of this data, and will not require futher calls to the database.

The constructors for the classes accept arrays as input, which allow for prepopulation of the data
store, for cases like the comments page, where an upload, its comments, and the comments' users
all must be loaded from the database.
*/

class Upload {
	private static $gdata = array();
	private $data;
	
	public function __construct($info) {
		/*
		 * info must be an id or aggregate array.  for more information about aggregate arrays,
		 * see this file's header comment.  the Upload class supports the following nested
		 * aggregate arrays:
		 * 		* User data with keys prefixed by user_
		 */
		if(is_array($info)) {
			if(!array_key_exists("id", $info))
				trigger_error("cannot prepopulate an Upload object without an id", E_USER_ERROR);
			
			// set up the object alias for this object's data in the global data structure
			if(!array_key_exists($info['id'], self::$gdata)) {
				// create if it doesn't already exist
				self::$gdata[$info['id']] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info['id']];
			
			/*
			 * strip out keys that start with user_ into their own array,
			 * to be used to prepopulate a member User object in this Upload object
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
			 * grab the userid from the data as well, if available.  this will help
			 * rescue the failure case where user data not including the userid
			 * is included in the incoming data, which would cause it to be discarded
			 * and an error generated.
			 */
			if(array_key_exists("userid", $this->data))
				$userarr["userid"] = $this->data["userid"];
				
			// if we found any user data, create a User object from it.
			if(count($userarr) > 0) {
				$this->data["uploader"] = new User($userarr);
			}
		}
		/* it's a little silly that this conditional is the best way I can
		 * think of to test if a variable of any type is losslessly castable
		 * to an int (or already an int).  is_int() should be smarter.
		 */
		else if(is_numeric($info) && $info == (int)$info) {
			if(!array_key_exists($info, self::$gdata)) {
				self::$gdata[$info] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info];
			$this->data['id'] = $info;
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	private function getUploadRow($element=false) {
		// if we haven't hit the db for this upload already, do it
		if(($element == false || !array_key_exists($element, $this->data)) &&
		   $this->data['exists'] === null) {
			// get information we need from the database.
			$sql = "SELECT * FROM offensive_uploads WHERE id = ".$this->data["id"];
			$result = tmbo_query($sql);
			
			// if the upload doesn't exist...  well what can we do?
			if(mysql_num_rows($result) != 1) {
				$this->data['exists'] = false;
			} else {
				$row = mysql_fetch_assoc($result);
				foreach($row as $key => $val) {
					$this->data[$key] = $val;
				}
				$this->data['exists'] = true;
			}
		}
		// fulfil the query
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_uploads table, but was not available",
			              E_USER_ERROR);
		}
		return $this->data['exists'];
	}
	
	public function exists() {
		return $this->getUploadRow();
	}
	
	public function expired() {
		if(!$this->exists()) return true;
		if($this->file() == '') return true;
		return false;
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
	
	public function status() {
		return $this->getUploadRow("status");
	}
	
	/*
	 * functions that deal with comment and vote information that are
	 * stored in the offensive_count_cache
	 */
	private function getCacheRow($element) {
		// try to close requests immediately if they are simple element queries
		if(!array_key_exists($element, $this->data)) {
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
		}
      
		if(array_key_exists($element, $this->data))
			return $this->data[$element];
		// if not, register an error.
		trigger_error("$element requested from offensive_count_cache table, but was not available",
		              E_USER_ERROR);
	}
	
	public function goods() {
		return is_numeric($this->getCacheRow("goods")) 
			? $this->getCacheRow("goods")
			: 0;
	}
	
	public function bads() {
		return is_numeric($this->getCacheRow("bads"))
			? $this->getCacheRow("bads")
			: 0;
	}
	
	public function tmbos() {
		return is_numeric($this->getCacheRow("tmbos"))
			? $this->getCacheRow("tmbos")
			: 0;
	}
	
	public function tiars() {
		return is_numeric($this->getCacheRow("tiars"))
			? $this->getCacheRow("tiars")
			: 0;
	}
	
	public function comments() {
		return is_numeric($this->getCacheRow("comments"))
			? $this->getCacheRow("comments")
			: 0;
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
				$sql = "SELECT u.* FROM users u
				        JOIN offensive_uploads up ON up.userid = u.userid
				        WHERE up.id = ".$this->id();
				
				$result = tmbo_query($sql);
				
				/* this error should be impossible.  if we ever hit it, we have a
				 * datastore inconsistency problem on our hands. */
				if(mysql_num_rows($result) !== 1) {
					trigger_error("user failure, user for this upload does not exist?", E_USER_ERROR);
				}
				
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
			
			$sql = "SELECT com.*,
			               u.userid AS user_userid,
			               u.username AS user_username,
			               u.created AS user_created,
			               u.account_status AS user_account_status,
			               u.timestamp AS user_timestamp
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
		        	WHERE id > ".$this->id()." AND type='".$this->type()."' AND status='normal'
					AND id != ".$this->id();
					/*
					 * this last conditional was added due to a bug in MySQL that would
					 * sometimes cause this query to return the current id as the next id.
					 */
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
		        	WHERE id < ".$this->id()." AND type='".$this->type()."' AND status='normal'
					AND id != ".$this->id();
					/*
					 * this last conditional was added due to a bug in MySQL that would
					 * sometimes cause this query to return the current id as the previous id.
					 */
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
	 * filtering code
	 */
	public function filtered() {
		return $this->squelched() ||
		       $this->filtered_nsfw() ||
		       $this->filtered_tmbo() ||
		       $this->filtered_bad();
	}
	
	public function squelched() {
		return (me() != false
			? me()->squelched($this->uploader())
			: false);
	}
	
	public function filtered_nsfw() {
		return (me() != false
			? me()->getPref("hide_nsfw") == 1 && $this->is_nsfw() == 1
			: false);
	}
	
	public function filtered_tmbo() {
		return (me() != false
			? me()->getPref("hide_tmbo") == 1 && $this->is_tmbo() == 1
			: false);
	}
	
	public function filtered_bad() {
		return (me() != false
			? me()->getPref("hide_bad") == 1
			  && $this->bads() > 10
			  && $this->bads() / 9 > $this->goods()
			: false);
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
		// XXX: eventually htmlFilename should reside inside this function, not callable from outside.
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
	private static $gdata = array();
	private $data = array();
	
	public function __construct($info) {
		$this->data = array();
		/*
		 * info must be an id or aggregate array.  for more information about aggregate arrays,
		 * see this file's header comment.  the Comment class supports the following nested
		 * aggregate arrays:
		 * 		* User data with keys prefixed by user_
		 */
		if(is_array($info)) {
			if(!array_key_exists("id", $info))
				trigger_error("cannot prepopulate a Comment object without an id", E_USER_ERROR);

			/* prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 * that might change if there are issues with location, yearbook, or posse
			 */
			if(!array_key_exists($info['id'], self::$gdata)) {
				self::$gdata[$info['id']] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info['id']];

			/* strip out keys that start with user_ into their own array,
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

			/* rescue the userid from the data as well, this will help
			 * avoid the failure case where user data not including the userid
			 * is included in the incoming data.
			 */
			if(array_key_exists("userid", $this->data)) {
				$userarr["userid"] = $this->data["userid"];
			}
			if(count($userarr) > 0) {
				$this->data["commenter"] = new User($userarr);
			}
		} else if(is_numeric($info) && $info == (int)$info) {
			if(!array_key_exists($info, self::$gdata)) {
				self::$gdata[$info] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info];
			$this->data = array("id" => $info);
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	private function getCommentRow($element=false) {
		// if we haven't hit the db for this upload already, do it
		if(($element == false || !array_key_exists($element, $this->data)) &&
		   $this->data['exists'] === null) {
			// get information we need from the database.
			$sql = "SELECT * FROM offensive_comments WHERE id = ".$this->data["id"];
			$result = tmbo_query($sql);
			
			// if the upload doesn't exist...  well what can we do?
			if(mysql_num_rows($result) != 1) {
				$this->data['exists'] = false;
			} else {
				$row = mysql_fetch_assoc($result);
				foreach($row as $key => $val) {
					$this->data[$key] = $val;
				}
				$this->data['exists'] = true;
			}
		}
		// fulfil the query
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_comments table, but was not available",
			              E_USER_ERROR);
		}
		return $this->data['exists'];
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
		/* the database requires sanitization and type enforcement:
		 *
		 * mysql> select distinct offensive from offensive_comments;
		 * +-----------+
		 * | offensive |
		 * +-----------+
		 * |         0 | 
		 * |         1 | 
		 * |      NULL | 
		 * +-----------+
		 * 3 rows in set (12.21 sec)
		 *
		 * thus this code is more clever than it really should need to.
		 */
		
		return (1 == $this->getCommentRow("offensive") ? true : false);
	}
	
	public function tiar() {
		/* the database requires sanitization and type enforcement:
		 *
		 * 	mysql> select distinct repost from offensive_comments;
		 *  +--------+
		 *  | repost |
		 *  +--------+
		 *  |   NULL | 
		 *  |      1 | 
		 *  |      0 | 
		 *  +--------+
		 *  3 rows in set (14.39 sec)
		 *
		 * thus this code is more clever than it really should need to.
		 */
		return (1 == $this->getCommentRow("repost") ? true : false);
	}
	
	public function timestamp() {
		return $this->getCommentRow("timestamp");
	}
	
	public function text() {
		return $this->getCommentRow("comment");
	}
	
	public function HTMLcomment() {
		$comment = $this->text();
		if($comment == "") return "";
		
		$comment = htmlEscape($comment);
		
		// URLS from tmbo domains should all point to the one the user is currently using
		$p = array("/http[s]?:\\/\\/([a-zA-z0-9]*\\.)*themaxx\\.com/i",
		           "/http[s]?:\\/\\/([a-zA-z0-9]*\\.)*tmbo\\.org/i",
		           "/http[s]?:\\/\\/([a-zA-z0-9]*\\.)*thismight\\.be/i",
		          );
		// whether the user is browsing http or https should also be respected
		$r = "http".
			 (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == "on" ? "s" : "").
			 "://".$_SERVER['SERVER_NAME'];
		$comment = preg_replace( $p, $r, $comment );
		// linkify all bare URLs
		$p = '/(http[s]?:\\/\\/[^\s<>]+)/i';
		$r = '<a href="$1" title="$1">$1</a>';
		$comment = preg_replace( $p, $r, $comment );

		$comment = nl2br($comment);
				
		return $comment;
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
				$sql = "SELECT u.* FROM users u, offensive_comments com 
				        WHERE u.userid = com.userid AND com.id = ".$this->data["id"];
				
				$result = tmbo_query($sql);
				if(mysql_num_rows($result) !== 1)
					trigger_error("user failure, user for this comment does not exist?", E_USER_ERROR);
				$this->data["commenter"] = new User(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["commenter"];
	}
}

class User {
	private static $gdata = array();
	private $data;
	
	/*
	 * info must be an id or aggregate array.  for more information about aggregate arrays,
	 * see this file's header comment.  the User class supports the following nested
	 * aggregate arrays:
	 * 		* none.
	 */
	public function __construct($info) {
		if(is_array($info)) {
			if(!array_key_exists("userid", $info))
				trigger_error("cannot prepopulate a User object without a userid", E_USER_ERROR);
			/*
			 * prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 * that might change if there are issues with location, yearbook, or posse
			 */
			if(!array_key_exists($info['userid'], self::$gdata)) {
				self::$gdata[$info['userid']] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info['userid']];
			
			foreach($info as $key => $val) {
				$this->data[$key] = $val;
			}
		} else if(is_numeric($info) && $info == (int)$info) {
			if(!array_key_exists($info, self::$gdata)) {
				self::$gdata[$info] = array('exists' => null);
			}
			$this->data =& self::$gdata[$info];
			$this->data['userid'] = $info;
		} else {
			trigger_error("type mismatch: constructor argument must be an integer or array", E_USER_ERROR);
		}
	}
	
	/* populate the internal data structure with information from the row
	 * in the users table belonging to this user.
	 */
	private function getUserRow($element=false) {
		// if we haven't hit the db for this upload already, do it
		if(($element == false || !array_key_exists($element, $this->data)) &&
		   $this->data['exists'] === null) {
			// get information we need from the database.
			$sql = "SELECT * FROM users WHERE userid = ".$this->data["userid"];
			$result = tmbo_query($sql);
			
			// if the upload doesn't exist...  well what can we do?
			if(mysql_num_rows($result) != 1) {
				$this->data['exists'] = false;
			} else {
				$row = mysql_fetch_assoc($result);
				foreach($row as $key => $val) {
					$this->data[$key] = $val;
				}
				$this->data['exists'] = true;
			}
		}
		
		// fulfil the query
		if($element) {
			if(array_key_exists($element, $this->data))
				return $this->data[$element];
			// if not, register an error.
			trigger_error("$element requested from offensive_comments table, but was not available",
			              E_USER_ERROR);
		}
		return $this->data['exists'];
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
	// id() is guaranteed to work for all User objects.
	// whether or not the data it returns is good can be determined with exists().
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
			$sql = "SELECT id FROM offensive_uploads WHERE type = 'avatar' AND status = 'normal' AND userid = ".$this->data["userid"].
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

	/*
	 * squelching
	 */
	// has this user squelched the target?
	public function squelched($target) {
		if(is_object($target)) {
			$target = $target->id();
		} else if(!is_numeric($target) || $target != (int)$target) {
			trigger_error("target must be a userid or User object", E_USER_ERROR);
		}
		
		if(!array_key_exists("squelched", $this->data)) {
			$this->data["squelched"] = array();
			$sql = "SELECT squelched FROM offensive_squelch WHERE userid=".$this->id();
			$result = tmbo_query($sql);

			while($row = mysql_fetch_array($result)) {
				$this->data["squelched"][] = $row["squelched"];
			}
		}
		
		return in_array($target, $this->data["squelched"]);
	}

	// squelch a user
	public function squelch($target) {
		// populate our local squelch cache
		$this->squelched(0);
		// make sure user exists and all that
		if(is_object($target)) {
			$target = $target->id();
		} else if(!is_numeric($target) || $target != (int)$target) {
			trigger_error("target must be a userid or User object", E_USER_ERROR);
		} else {
			$obj = new User($target);
			if(!$obj->exists()) {
				trigger_error("target($target) does not exist", E_USER_ERROR);
			}
		}
		
		$sql = "INSERT INTO offensive_squelch(userid, squelched) VALUES ( ".$this->id().", $target )";
		tmbo_query( $sql );
		
		foreach($this->data["squelched"] as $squelchee) {
			if($squelchee == $target) return;
		}
		$this->data["squelched"][] = $target;
	}
	
	// unsquelch a user
	public function unsquelch($target) {
		// populate our local squelch cache
		$this->squelched(0);
		// make sure user exists and all that
		if(is_object($target)) {
			$target = $target->id();
		} else if(!is_numeric($target) || $target != (int)$target) {
			trigger_error("target must be a userid or User object", E_USER_ERROR);
		} else {
			$obj = new User($target);
			if(!$obj->exists()) {
				trigger_error("target($target) does not exist", E_USER_ERROR);
			}
		}
		
		$sql = "DELETE FROM offensive_squelch WHERE userid=".$this->id()." AND squelched=$target";
		tmbo_query($sql);
		
		foreach($this->data["squelched"] as $key => $squelchee) {
			if($squelchee == $target) unset($this->data["squelched"][$key]);
		}
	}
	
	/*
	 * preferences
	 */
	public function getPref($name) {
		if(!array_key_exists("prefs", $this->data)) {
			$this->data["prefs"] = array();
			$sql = "SELECT prefname, value FROM user_preferences WHERE userid = ".$this->id();
			$result = tmbo_query($sql);
			
			while(false !== ($row = mysql_fetch_assoc($result))) {
				$this->data["prefs"][$row["prefname"]] = $row["value"];
			}
		}
		
		if(!array_key_exists("$name", $this->data["prefs"]))
			return false;
		return $this->data["prefs"]["$name"];
	}
	
	public function setPref($name, $value) {
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			trigger_error("$name is not a valid preference name!", E_USER_ERROR);
		if(!$this->exists())
			trigger_error("user(".$this->id().") does not exist", E_USER_ERROR);
		
		if(strlen($value) > 0) {
			// check if preference is already set
			if($this->getPref($name) === false) {
				$sql = "INSERT INTO user_preferences(userid, prefname, value)
			        	VALUES(".$this->id().", '$name', '".sqlEscape($value)."')";
			} else {
				$sql = "UPDATE user_preferences SET value = '".sqlEscape($value)."'
			        	WHERE userid = ".$this->id()." AND prefname = '$name'";
			}
		} else {
			// if we're deleting, and the pref doesn't exist anyway, we're done.
			if($this->getPref($name) === false) {
				return;
			}
			// if the value is blank, we're deleting
			$sql = "DELETE FROM user_preferences WHERE prefname = '$name' AND userid = ".$this->id();
		}
		
		// write to cache
		$this->data["prefs"]["$name"] = $value;
		
		// write through cache
		tmbo_query($sql);
	}
}

?>