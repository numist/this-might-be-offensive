<?

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
		   (!array_key_exists('exists', $this->data) || $this->data['exists'] === null)) {
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
		return $this->getCommentRow(__FUNCTION__);
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
		 * thus this code is more clever than it really should need to be.
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
		 * thus this code is more clever than it really should need to be.
		 */
		
		return (1 == $this->getCommentRow("repost") ? true : false);
	}
	
	public function timestamp() {
		return $this->getCommentRow(__FUNCTION__);
	}
	
	public function text() {
		$comment = $this->getCommentRow("comment");
		if($this->squelched() && strlen($comment) > 0) {
			$comment = "yes, there's a comment. no, you can't see it.";
		}
		return $comment;
	}
	
	public function HTMLcomment() {
		$comment = $this->text();
		if($comment == "") return "";

		if($this->squelched()) {
			return "<div class='squelched'>[ squelched ]</div>";
		}

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

		if($this->blocked()) {
			return "<span class='blocked'>$comment</span>";
		}

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
	
	public function blocked() {
		return me()->blocked($this->commenter());
	}
	
	public function squelched() {
		return me()->squelched($this->commenter());
	}

	public function channels() {
		return array(
			"/uploads/" . $this->upload()->id(),
			"/users/" . $this->commenter()->id()
		);
	}

	public function api_data($include_html = false) {
		$ret = array(
			"fileid" => $this->upload()->id(),
			"filename" => $this->upload()->filename(),
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

		if($include_html) {
			ob_start();
			$comment = $this;
			require('offensive/templates/comment.inc');
			$ret["html"] = ob_get_contents();
			ob_end_clean();
		}

		return $ret;
	}
}

?>
