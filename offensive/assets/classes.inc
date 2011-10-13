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
				} else if($key != "exists") {
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
		else if(is_intger($info)) {
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
		return is_intger($this->getCacheRow("goods")) 
			? $this->getCacheRow("goods")
			: 0;
	}
	
	public function bads() {
		return is_intger($this->getCacheRow("bads"))
			? $this->getCacheRow("bads")
			: 0;
	}
	
	public function tmbos() {
		return is_intger($this->getCacheRow("tmbos"))
			? $this->getCacheRow("tmbos")
			: 0;
	}
	
	public function tiars() {
		return is_intger($this->getCacheRow("tiars"))
			? $this->getCacheRow("tiars")
			: 0;
	}
	
	public function comments() {
		return is_intger($this->getCacheRow("comments"))
			? $this->getCacheRow("comments")
			: 0;
	}
	
	public function last_active() {
		$last_active = $this->getCacheRow("last_active");
		return ($last_active == null ? $this->timestamp() : $last_active);
	}
	
	public function subscribed() {
		if(!array_key_exists('subscribed', $this->data)) {
			$sql = "SELECT * FROM offensive_subscriptions
				WHERE userid=".me()->id()." AND fileid=".$this->id();
			$this->data['subscribed'] = mysql_num_rows(tmbo_query($sql)) > 0 ? true : false;
		}
		
		return (bool)$this->data['subscribed'];
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
				$this->data["uploader"] = new User($this->getUploadRow("userid"));
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
		if(!array_key_exists("thecomments", $this->data)) {
			$this->data["thecomments"] = array();
			
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
				$this->data["thecomments"][] = new Comment($row);
			}
		}
		
		return $this->data["thecomments"];
	}
	
	/*
	 * upload slinging code
	 */
	public function next() {
		if(!array_key_exists("next", $this->data)) {
			
			$sql = "SELECT * FROM offensive_uploads WHERE id > ".$this->id()." AND type='".$this->type()."' AND status='normal'
					ORDER BY id ASC LIMIT 1";	
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) == 0) {
				$this->data["next"] = false;
			} else {
				$this->data["next"] = new Upload(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["next"];
	}
	
	public function next_filtered() {
		if(!me()) return $this->next();
		
		if(!array_key_exists("next_filtered", $this->data)) {			
			$tmbo_sql = me()->getPref("hide_tmbo") ? " AND NOT tmbo " : "";
			$nsfw_sql = me()->getPref("hide_nsfw") ? " AND NOT nsfw " : "";
        
			$sql = "SELECT * FROM offensive_uploads WHERE id > {$this->id()} AND type='{$this->type()}' AND status='normal'
					$tmbo_sql
					$nsfw_sql
					AND userid NOT IN (SELECT squelched FROM offensive_squelch WHERE userid = ".me()->id()." )
					ORDER BY id ASC LIMIT 1";
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) == 0) {
				$this->data["next_filtered"] = false;
			} else {
				$this->data["next_filtered"] = new Upload(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["next_filtered"];
	}
	
	public function prev() {
		if(!array_key_exists("prev", $this->data)) {
      
			$sql = "SELECT * FROM offensive_uploads WHERE id < ".$this->id()." AND type='".$this->type()."' AND status='normal'
					ORDER BY id DESC LIMIT 1";
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) == 0) {
				$this->data["prev"] = false;
			} else {
				$this->data["prev"] = new Upload(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["prev"];
	}

	public function prev_filtered() {
		if(!me()) return $this->prev();

		if(!array_key_exists("prev_filtered", $this->data)) {
			$tmbo_sql = me()->getPref("hide_tmbo") ? " AND NOT tmbo " : "";
			$nsfw_sql = me()->getPref("hide_nsfw") ? " AND NOT nsfw " : "";
      
			$sql = "SELECT * FROM offensive_uploads WHERE id < {$this->id()} AND type='{$this->type()}' AND status='normal'
          			$tmbo_sql
          			$nsfw_sql
					AND userid NOT IN (SELECT squelched FROM offensive_squelch WHERE userid = ".me()->id()." )
					ORDER BY id DESC LIMIT 1";
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) == 0) {
				$this->data["prev_filtered"] = false;
			} else {
				$this->data["prev_filtered"] = new Upload(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["prev_filtered"];
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
	
	private function basePath($timestamp=null, $nopath=false) {
		if($timestamp == null) $timestamp = $this->timestamp();
		
		if(!is_intger($timestamp))
			$timestamp = strtotime($timestamp);
		
		$year = date( "Y", $timestamp );
		$month = date( "m", $timestamp );
		$day = date( "d", $timestamp );
		
		return ($nopath ? "" : get_include_path()."/")."offensive/uploads/$year/$month/$day/".$this->type()."/";
	}
		
	private function fileConform($path) {
		if($path == '' || !file_exists($path)) return false;
		
		$target = $this->basePath();
		ensureDirExists($target);
		$target .= $this->id()."_".$this->filename();
		
		// if timestamp is numeric, we want to move, not copy
		if(!rename($path, $target)) {
			trigger_error("rename($path, $target) failed", E_USER_WARNING);
			return false;
		}
		
		trigger_error("CONFORM: moved $path to $target", E_USER_NOTICE);
		return true;
	}
	
	public function file() {
		$id = $this->id();
		$filename = $this->filename();
				
		// uploads with files only
		if($this->type() == 'topic') {
			return '';
		}

		$retname = $this->basePath().$id."_".$filename;

		// if there are still files somewhere following the old naming scheme, this will flush them out.
		if(file_exists($this->basePath().$filename)) {
			rename($this->basePath().$filename, $retname);
		}

		if(file_exists($retname)) {
			return $retname;
		}
		
		/**********************************
		 * UH OH!  image not found!
		 * ignore the man behind the curtain
		 * maybe a backup restoration caused unicodes to break,
		 * or caused things to shift into yesterday.  or tomorrow.
		 * sigh.
		 */
		if(count(glob($this->basePath().$id."_*")) == 1) {
			list($ffilename) = glob($this->basePath().$id."_*");
			$filename = basename($ffilename);
			$filename = substr($ffilename, strpos($ffilename, "_") + 1);
			$sql = "UPDATE offensive_uploads SET timestamp=timestamp, filename = '".sqlEscape($filename)."' WHERE id = ".$this->id();
			tmbo_query($sql);
			return $ffilename;
		}
		
		$timestamp = strtotime($this->timestamp());
			
		$yesterday = $timestamp - 86400;
		$retname = $this->basePath($yesterday).$id."_".$filename;
		if($this->fileConform($retname)) return $retname;
			
		$tomorrow = $timestamp + 86400;
		$retname = $this->basePath($tomorrow).$id."_".$filename;
		if($this->fileConform($retname)) return $retname;

		return '';
	}
	
	public function URL() {
		// uploads with files only
		// if there's no file, why bother with a URL?
		if($this->type() == 'topic'
		   || $this->file() == '') {
			return '';
		}

		return "/".$this->basePath($this->timestamp(), true)
		       .$this->id()."_".rawurlencode($this->filename());
	}
	
	private function thumbConform($path) {
		if($path == '' || !file_exists($path)) return false;
		
		$target = $this->basePath()."thumbs/";
		ensureDirExists($target);
		$target .= "th".$this->id().($this->type() == "audio" ? "" : getFileExtension($path));

		if(!rename($path, $target)) {
			trigger_error("rename($path, $target) failed", E_USER_WARNING);
		}

		trigger_error("CONFORM: moved $path to $target", E_USER_NOTICE);
		return true;
	}
	
	public function thumb() {
		$id = $this->id();
		$timestamp = $this->timestamp();
		$type = $this->type();
		$ext = getFileExtension($this->filename());
		$thumbpath = "thumbs/th$id".($type == "audio" ? "" : $ext);

		// files only
		if($type == 'topic') return '';

		$retname = $this->basePath().$thumbpath;

		if(file_exists($retname)) {
			return $retname;
		}
		
		/**********************************
		 * UH OH!  image not found!
		 * ignore the man behind the curtain
		 * what about yesterday?  or tomorrow?
		 */
		
		$timestamp = strtotime($timestamp);
			
		$yesterday = $timestamp - 86400;
		$retname = $this->basePath($yesterday).$thumbpath;
		if(Upload::thumbConform($retname)) return $retname;
			
		$tomorrow = $timestamp + 86400;
		$retname = $this->basePath($tomorrow).$thumbpath;
		if(Upload::thumbConform($retname)) return $retname;

		if($type == "audio") {
			return "";
		}
		
		// still no good :( … is there a big image we can create a thumb from?
		$bigfile = $this->file();
		if($bigfile == '') {
			return '';
		}
		
		$retname = $this->basePath().$thumbpath;
		
		// man, I sure hope this doesn't happen often.
		// this will probably hurt if we get dogpiled, but this is a pretty uncommon code path
		$tmpfile = get_include_path()."/offensive/quarantine/$id".rand().$ext;
		copy($bigfile, $tmpfile);
        
		$cmd = "convert -resize 100x100 $tmpfile $tmpfile";
		$err = shell_exec($cmd);
		if(strlen($err > 0)) {
			trigger_error("ImageMagick failed: $err", E_USER_WARNING);
			return '';
		}
		ensureDirExists(dirname($retname));
		rename($tmpfile, $retname);
        
		trigger_error("CONFORM: resized $bigfile to $retname", E_USER_NOTICE);
        
		return $retname;
	}
	
	public function thumbURL() {
		// images only
		// if the file or thumb don't exist, why bother?
		if($this->type() == 'topic' || $this->file() == '' || $this->thumb() == '') {
			return "/offensive/graphics/previewNotAvailable.gif";
		}
		
		return "/".$this->basePath(null, true)."thumbs/th".$this->id().
		       ($this->type() == "audio" ? "" : getFileExtension($this->filename()));
	}
	
	public function commentLabel($showvotes=true) {
		$comments = $this->comments();

		$retval = ($comments == 0 ? "no" : $comments)." comment".($comments == 1 ? "" :"s");
		if(!$showvotes) return $retval;
		$retval .= " (+".$this->goods()." -".$this->bads()
		        . ($this->tmbos() > 0 ? " x".$this->tmbos() : "").")";
		return $retval;
	}

	public function htmlFilename($showtype=false) {
		$tags = array();

		if($showtype) {
			$tags[] = $this->type().":";
		}

		if($this->is_nsfw()) {
			$tags[] = "[nsfw]";
		}

		if($this->is_tmbo()) {
			$tags[] = "[tmbo]";
		}

		// TODO: consume tags from image name

		return (count( $tags ) > 0 ? "<span class=\"tags\">" . implode( "", $tags ) . "</span> " : "")
		       .htmlEscape($this->filename());
	}

	// API support
	public function api_data() {

		$ret = array(
				"id" => $this->id(),
				"userid" => $this->uploader()->id(),
				"filename" => $this->filename(),
				"timestamp" => $this->timestamp(),
				"nsfw" => $this->is_nsfw(),
				"tmbo" => $this->is_tmbo(),
				"type" => $this->type(),
				"username" => $this->uploader()->username(),
				"subscribed" => $this->subscribed(),
				"vote_good" => $this->goods(),
				"vote_bad" => $this->bads(),
				"vote_tmbo" => $this->tmbos(),
				"vote_repost" => $this->tiars(),
				"comments" => $this->comments(),
				"filtered" => $this->filtered()
			);
		if($this->type() != "topic" && $this->file() != '') {
			$ret["link_file"] = $this->URL();
		}
		if(($this->type() == "image" || $this->type() == "avatar")
		   && $this->file() != "") {
			$arr = getimagesize($this->file());
			if($arr === false) { trigger_error($this->file()." is not a valid image!?", E_USER_ERROR); }
			
			$ret["width"] = $arr[0];
			$ret["height"] = $arr[1];
			
			$ret["link_thumb"] = $this->thumbURL();
			
			$arr = getimagesize($this->thumb());
			
			$ret["thumb_width"] = $arr[0];
			$ret["thumb_height"] = $arr[1];
		}
		
		return $ret;
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
		 *		* Uplaod data with keys prefixed by upload_
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

			/* strip out keys that start with user_ or upload_ into their own array,
			 * to be used to prepopulate a User or Upload object to go with this Comment object
			 */
			$userarr = array();
			$uplarr = array();
			foreach($info as $key => $val) {
				if(strpos($key, "user_") === 0) {
					$userarr[substr($key, 5)] = $val;
				} else if(strpos($key, "upload_") === 0) {
					$uplarr[substr($key, 7)] = $val;
				} else if($key != "exists"){
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
			if(count($userarr) > 0 && array_key_exists("userid", $userarr)) {
				$this->data["commenter"] = new User($userarr);
			}
			
			/* rescue the fileid as well, this will help avoid the failure case
			 * where file data not including the fileid
			 * is included in the incoming data
			 */
			if(array_key_exists("fileid", $this->data)) {
				$uplarr["id"] = $this->data["fileid"];
			}
			if(count($uplarr) > 0 && array_key_exists("id", $uplarr)) {
				$this->data["upload"] = new Upload($uplarr);
			}
			
		} else if(is_intger($info)) {
			// there's not a lot to do if the argument is just an int.
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
		$r = "https://".$_SERVER['SERVER_NAME'];
		$comment = preg_replace( $p, $r, $comment );


		// linkify all bare URLs
		$comment = " ".$comment." ";
		$p = '/(http[s]?:\\/\\/[^\s<>]+)/i';
		$r = '<a href="$1" rel="nofollow">$1</a>';
		$comment = preg_replace( $p, $r, $comment );
		$comment = trim($comment);

		$comment = nl2br($comment);

		return $comment;
	}
	
	public function upload() {
		if(!array_key_exists("upload", $this->data)) {
			if(array_key_exists("fileid", $this->data)) {
				/*
				 * if we already have a userid, the User class can do the
				 * database work, it already knows how.
				 */
				$this->data["upload"] = new Upload($this->getCommentRow("fileid"));
			} else {
				/*
				 * if we have to go to the database anyway, we may as well
				 * grab the entire user row, to prevent a duplicate database call
				 * from the User class
				 */
				$sql = "SELECT up.* FROM offensive_uploads up
				        JOIN offensive_comments com ON up.id = com.fileid
				        WHERE com.id = ".$this->id();
				
				$result = tmbo_query($sql);
				
				/* this error should be impossible.  if we ever hit it, we have a
				 * datastore inconsistency problem on our hands. */
				if(mysql_num_rows($result) !== 1) {
					trigger_error("upload failure, upload for this comment does not exist?", E_USER_ERROR);
				}
				
				$this->data["upload"] = new Upload(mysql_fetch_assoc($result));
			}
		}
		
		return $this->data["upload"];
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

	public function api_data() {
		$ret = array(
			"fileid" => $this->upload()->id(),
			"id" => $this->id(),
			"timestamp" => $this->timestamp(),
			"userid" => $this->commenter()->id(),
			"username" => $this->commenter()->username()
		);

		if($this->vote()) {
			$ret["vote"] = $this->vote();
		}
		if($this->tmbo()) {
			$ret["tmbo"] = $this->tmbo();
		}
		if($this->tiar()) {
			$ret["tiar"] = $this->tiar();
		}
		if($this->text()) {
			$ret["comment"] = $this->text();
		}

		return $ret;
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
				if($key != "exists") {
					$this->data[$key] = $val;
				}
			}
		} else if(is_intger($info)) {
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
	public function userid() {
		return $this->id();
	}
	
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
	
	public function last_login_ip() {
		return $this->getUserRow("last_login_ip");
	}
	
	public function referred_by() {
		if($this->getUserRow("referred_by") == $this->data["userid"]) {
			return false;
		}
		return new User($this->getUserRow("referred_by"));
	}
	
	/*
	 * token data
	 */
	public function token() {
		if(array_key_exists("token", $this->data) &&
		   !is_object($this->data["token"])) {
			$this->data["token"] = new Token($this->data["token"]);
		} else {
			assert('me()->id() == $this->id()');
			$this->data["token"] = new Token();
		}
		return $this->data["token"];
	}
	
	/*
	 * advanced user row stuff
	 */
	
	public function posse() {
		if(!array_key_exists("posse", $this->data)) {
			$this->data["posse"] = array();
			$sql = "SELECT * FROM users WHERE referred_by = ".$this->data["userid"]. "
			        AND userid != ".$this->data["userid"];
			$result = tmbo_query($sql);
			if(mysql_num_rows($result) != 0) {
				while(false !== ($row = mysql_fetch_assoc($result))) {
					if($row['userid'] != $this->id()) {
						$this->data["posse"][] = new User($row);
					}
				}
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
			        FROM maxxer_locations WHERE mapversion = 'google' AND userid = ".$this->data["userid"];
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
	public function avatar() {
		return $this->yearbook();
	}
	
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
		} else if(!is_intger($target)) {
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
		} else if(!is_intger($target)) {
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
		} else if(!is_intger($target)) {
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
	 * string slinging code
	 */

	public function htmlUsername() {
		$color = null;
		switch($this->status()) {
			case "locked":
				$color = "red";
				break;
			case "awaiting activation":
				$color = "green";
				break;
		}
		$ret = "<a class=\"userLink\" href=\"/offensive/?c=user&userid=".$this->id()."\">";
		if($color) {
			$ret .= "<span style=\"color: $color;\">".$this->username()."</span>";
		} else {
			$ret .= $this->username();
		}

		return $ret."</a>";
	}

	/*
	 * preferences
	 */
	public function getPref($name) {
		global $redislink;
		return $redislink->get($this->id().':'.$name);
	}
	
	public function setPref($name, $value) {
		global $redislink;
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			trigger_error("$name is not a valid preference name!", E_USER_ERROR);
		if(!$this->exists())
			trigger_error("user(".$this->id().") does not exist", E_USER_ERROR);
		
		if(strlen($value) > 0) {
			$redislink->set($this->id().':'.$name, $value);
		} else {
			$redislink->delete($this->id().':'.$name);
		}
	}

	public function api_data() {
		$ret = array(
				"posse" => count($this->posse()),
				"yearbook" => ($this->yearbook() !== false ? true : false),
				"userid" => $this->id(),
				"username" => $this->username(),
				"created" => $this->created(),
				"account_status" => $this->status(),
				"timestamp" => $this->timestamp(),
			);
		
		if($this->referred_by()){
			$ret["referer_id"] = $this->referred_by()->id();
			$ret["referer"] = $this->referred_by()->username();
		}
		
		if($this->status() == "admin") {
			$ret["ip"] = $this->last_login_ip();
			$ret["email"] = $this->email();
		}
		
		return $ret;
	}
}

class Link {
	// pagination links (search query_string)
	static function content($page) {
		assert('is_string($page)');
		return "/offensive/?c=$page";
	}

	static function upload($upload) {
		if(!is_object($upload)) {
			$upload = new Upload($upload);
		}

		if($upload->type() == "topic") {
			return Link::thread($upload);
		}

		return "/offensive/pages/pic.php?id=".$upload->id();
	}

	static function comment($comment) {
		if(!is_object($comment)) {
			$comment = new Comment($comment);
		}

		return Link::thread($comment->upload())."#".$comment->id();
	}

	static function user($user) {
		if(!is_object($user)) {
			$user = new User($user);
		}

		return Link::content("user")."&userid=".$user->id();
	}

	static function thread($upload) {
		if(!is_object($upload)) {
			$upload = new Upload($upload);
		}
		return Link::content("comments")."&fileid=".$upload->id();
	}

	static function file($upload) {
		if(!is_object($upload)) {
			$upload = new Upload($upload);
		}
		assert('$upload->type() != "topic"');

		return $upload->URL();
	}

	static function rss($type) {
		assert('in_array($type, array("audio", "zip", "pic", "discuss"))');
        $token = new Token(" rss");

		return "/offensive/".$type."_rss.php?token=".$token->tokenid();
	}
}

class Token {
	private static $gdata = array();
	private $data;

	/*
	 * info must be a tokenid or row from tokens table.
	 */
	public function __construct($info=null) {
		if($info === null) {
			$info = " tmbo";
		}
		
		if(is_array($info)) {
			/*
			 * the caller has passed an array that represents a row from the tokens table.
			 */
			if(!array_key_exists("tokenid", $info))
				trigger_error("cannot prepopulate a Token object without a tokenid", E_USER_ERROR);
			/*
			 * prepopulate with everything.
			 * this trusts other portions of the code to DO THE RIGHT THING
			 */
			if(!array_key_exists("tokenid:".$info["tokenid"], self::$gdata)) {
				self::$gdata["tokenid:".$info["tokenid"]] = array();
			}
			$this->data =& self::$gdata["tokenid:".$info["tokenid"]];

			foreach($info as $key => $val) {
				if($key != "exists") {
					$this->data[$key] = $val;
				}
			}
		} else if(is_string($info)) {
			if(strlen($info) < 16) {
				/*
				 * get a token by application
				 */
				assert('me()');
				if(!array_key_exists("app:$info", self::$gdata)) {
					self::$gdata["app:$info"] = array("userid" => me()->id(), "issued_to" => $info);
				}
				$this->data =& self::$gdata["app:$info"];
			} else {
				/*
				 * get a token by id
			 	 */
				if(!array_key_exists("tokenid:".$info, self::$gdata)) {
					self::$gdata["tokenid:".$info] = array("tokenid" => $info);
				}
				$this->data =& self::$gdata["tokenid:".$info];
			}
		} else {
			trigger_error("type mismatch: constructor argument must be a tokenid(string), array of token rows, or null", E_USER_ERROR);
		}
	}
	
	private function getTokenRow($element) {
		// if we haven't hit the db for this upload already, do it
		if(!array_key_exists($element, $this->data)) {
			// get information we need from the database.
			if(array_key_exists("tokenid", $this->data)) {
				$sql = "SELECT * FROM tokens WHERE tokenid = \"".sqlEscape($this->data["tokenid"])."\"";
			} else {
				assert('array_key_exists("userid", $this->data)');
				assert('array_key_exists("issued_to", $this->data)');
				$sql = "SELECT * FROM tokens WHERE issued_to = \"".sqlEscape($this->data["issued_to"])."\" AND userid = ".$this->data["userid"];
			}
			$result = tmbo_query($sql);
			
			// if the upload doesn't exist...  well what can we do?
			if(mysql_num_rows($result) != 1) {
				// there should never be more than one result for a token that matches either
				// query of "userid issued by ' tmbo'" or "tokenid".
				assert('mysql_num_rows($result) == 0');
				if(!array_key_exists("tokenid", $this->data) &&
				   array_key_exists("userid", $this->data) &&
				   array_key_exists("issued_to", $this->data)) {
					$token = core_createtoken($this->data["issued_to"]);
					$this->data =& self::$gdata["tokenid:".$token->tokenid()];
					$this->data['exists'] = true;
				} else {
					$this->data['exists'] = false;
				}
			} else {
				$row = mysql_fetch_assoc($result);
				foreach($row as $key => $val) {
					$this->data[$key] = $val;
				}
				$this->data['exists'] = true;
			}
		}
		
		// fulfil the query
		if(array_key_exists($element, $this->data)) {
			return $this->data[$element];
		}
		// if not, register an error.
		trigger_error("$element requested from tokens table, but was not available.",
		              E_USER_ERROR);
	}
	
	public function exists() {
		return $this->getTokenRow("exists");
	}
	
	public function id() {
		return $this->tokenid();
	}
	
	public function tokenid() {
		return $this->getTokenRow("tokenid");
	}
	
	public function userid() {
		return $this->getTokenRow("userid");
	}
	
	public function issued_to() {
		return $this->getTokenRow("issued_to");
	}
	
	public function issue_date() {
		return $this->getTokenRow("issue_date");
	}
	
	public function last_used() {
		return $this->getTokenRow("last_used");
	}
	
	public function reset_validity() {
		unset($this->data["exists"]);
	}
	
	public function readonly() {
	    return ($this->issued_to() == " rss");
	}
	
	public function api_data() {
		$ret = array(
				"tokenid" => $this->tokenid(),
				"userid" => $this->userid(),
				"issued_to" => $this->issued_to(),
				"issue_date" => $this->issue_date(),
				"last_used" => $this->last_used()
			);
		
		return $ret;
	}
}

?>
