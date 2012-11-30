<?

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
				throw new Exception("cannot prepopulate a User object without a userid");
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
			throw new Exception("type mismatch: constructor argument must be an integer or array");
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
			throw new Exception("$element requested from offensive_comments table, but was not available");
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
		return $this->getUserRow(__FUNCTION__);
	}
	
	public function email() {
		return $this->getUserRow(__FUNCTION__);
	}
	
	public function created() {
		return $this->getUserRow(__FUNCTION__);
	}
	
	public function status() {
		return $this->getUserRow("account_status");
	}
	
	public function timestamp() {
		return $this->getUserRow(__FUNCTION__);
	}
	
	public function last_login_ip() {
		return $this->getUserRow(__FUNCTION__);
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
			$locs = core_getlocation(array("userid" => $this->id()));
			
			if(count($locs) !== 1)
				$this->data["location"] = false;
			else
				$this->data["location"] = array("latitude" => $locs[0]["latitude"],
				                                "longitude" => $locs[0]["longitude"],
				                                "timestamp" => $locs[0]["timestamp"]);
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
			throw new Exception("target must be a userid or User object");
		}
		
		return in_array($target, $this->squelched_list());
	}
	
	private function populate_squelched() {
		if(!array_key_exists("squelched", $this->data)
		|| !array_key_exists("squelched_by", $this->data)) {
			
			// data consistency
			assert('!array_key_exists("squelched", $this->data)');
			assert('!array_key_exists("squelched_by", $this->data)');
			$this->data["squelched"] = array();
			$this->data["squelched_by"] = array();

			$sql = "SELECT userid,squelched FROM offensive_squelch WHERE userid=".$this->id()." OR squelched=".$this->id();
			$result = tmbo_query($sql);

			while($row = mysql_fetch_array($result)) {
				if($row["userid"] == $this->id()) {
					$this->data["squelched"][] = $row["squelched"];
				} else {
					$this->data["squelched_by"][] = $row["userid"];
				}
			}
		}
	}

	public function squelched_list() {
		$this->populate_squelched();
		return $this->data["squelched"];
	}
		
	public function squelched_by_list() {
		$this->populate_squelched();
		return $this->data["squelched_by"];
	}

	public function blocked($target) {
		if(is_object($target)) {
			$target = $target->id();
		}
		
		return $this->squelched($target) || in_array($target, $this->squelched_by_list());
	}

	// squelch a user
	public function squelch($target) {
		if(TMBO::readonly()) return;
		
		// sanity check
		if(me()->id() != $this->id() && me()->status() != "admin") {
			throw new Exception(me()->username()." can not squelch on ".$this->username()."'s behalf!");
		}
		
		// is this redundant?
		if($this->squelched($target)) {
			return;
		}
		
		// make sure user exists and all that
		if(is_object($target)) {
			$target = $target->id();
		} else if(!is_intger($target)) {
			throw new Exception("target must be a userid or User object");
		} else {
			$obj = new User($target);
			if(!$obj->exists()) {
				throw new Exception("target($target) does not exist", 404);
			}
		}
		
		$sql = "INSERT INTO offensive_squelch(userid, squelched) VALUES ( ".$this->id().", $target )";
		tmbo_query( $sql );
		$this->data["squelched"][] = $target;
	}
	
	// unsquelch a user
	public function unsquelch($target) {
		if(TMBO::readonly()) return;
		
		// sanity check
		if(me()->id() != $this->id() && me()->status() != "admin") {
			throw new Exception(me()->username()." can not squelch on ".$this->username()."'s behalf!");
		}
		
		// is this redundant?
		if(!$this->squelched($target)) {
			return;
		}
		
		// make sure user exists and all that
		if(is_object($target)) {
			$target = $target->id();
		} else if(!is_intger($target)) {
			throw new Exception("target must be a userid or User object");
		} else {
			$obj = new User($target);
			if(!$obj->exists()) {
				throw new Exception("target($target) does not exist", 404);
			}
		}
		
		$sql = "DELETE FROM offensive_squelch WHERE userid=".$this->id()." AND squelched=$target";
		tmbo_query($sql);
		unset($this->data["squelched"][$target]);
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
		$ret = "<a class=\"userLink\" href=\"".Link::user($this)."\">";
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
		global $redis;
		
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			throw new Exception("$name is not a valid preference name!", 400);
		
		$value = $redis->get($this->id().':'.$name);
		
		if($value == "" && array_key_exists($name, $prefdefaults)) {
			return $prefdefaults[$name];
		}
		
		return $value;
	}
	
	public function setPref($name, $value) {
		global $redis;
		
		if(TMBO::readonly()) return;
		
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			throw new Exception("$name is not a valid preference name!", 400);
		if(!$this->exists())
			throw new Exception("user(".$this->id().") does not exist", 404);
		
		if(strlen($value) > 0) {
			$redis->set($this->id().':'.$name, $value);
		} else {
			$redis->del($this->id().':'.$name);
		}
	}

	/*
	 * subscriptions
	 */
	public function subscribed($upload) {
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);

		$sql = "SELECT * FROM offensive_subscriptions
						WHERE userid={$this->id()} AND fileid={$upload->id()}";

		// memoize self
		if(me() && me()->id() == $this->id()) {
			if(!array_key_exists("subscribed:".$upload->id(), $this->data)) {
				$this->data["subscribed:".$upload->id()] = mysql_num_rows(tmbo_query($sql)) > 0 ? true : false;
			}
			return $this->data["subscribed:".$upload->id()];
		}
		
		return mysql_num_rows(tmbo_query($sql)) > 0 ? true : false;
	}

	public function subscribe($upload) {
		if(TMBO::readonly()) return false;
		
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		// check if the user is already subscribed
		if($this->subscribed($upload)) return true;

		// ensure file exists
		if(!$upload->exists()) return false;

    // finally, subscribe
		$sql = "INSERT INTO offensive_subscriptions (userid, fileid) VALUES ( {$this->id()}, {$upload->id()} )";
		tmbo_query($sql);
		
		// reset memoization
		if(me() && me()->id() == $this->id()) {
			$this->data["subscribed:".$upload->id()] = true;
		}
		
		return true;
	}
	
	public function unsubscribe($upload) {
		if(TMBO::readonly()) return false;
		
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		// check if the user is already unsubscribed
		if(!$this->subscribed($upload)) return true;

		// finally, unsubscribe
		$sql = "DELETE FROM offensive_subscriptions WHERE userid={$this->id()} AND fileid={$upload->id()}";
		tmbo_query($sql);
		
		// reset memoization
		if(me() && me()->id() == $this->id()) {
			$this->data["subscribed:".$upload->id()] = false;
		}
		
		return true;
	}
	
	public function clearSubscription($upload) {
		if(TMBO::readonly()) return false;
		
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		$sql = "UPDATE offensive_subscriptions SET commentid = NULL WHERE userid={$this->id()} and fileid={$upload->id()}";
		$result = tmbo_query( $sql );

		if(mysql_affected_rows() < 1) {
			return false;
		}

		publishChange("reset_subscription", array("/private/users/{$this->id()}"), $upload->api_data());
		return true;
	}

	/*
	 * permissions
	 */
	public function alreadyVoted($upload) {
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		$sql = "SELECT vote AS thecount FROM offensive_comments 
		        WHERE fileid={$upload->id()} AND userid={$this->id()} AND vote LIKE 'this%'";
		
		// memoize self
		if(me() && me()->id() == $this->id()) {
			if(!array_key_exists(__FUNCTION__.":".$upload->id(), $this->data)) {
				$this->data[__FUNCTION__.":".$upload->id()] = (mysql_num_rows(tmbo_query($sql)) > 0);
			}
			return $this->data[__FUNCTION__.":".$upload->id()];
		}
		return (mysql_num_rows(tmbo_query($sql)) > 0);
	}

	public function canVote($upload) {
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		// no need to memoize, this method is not responsible for any new data
		if($upload->uploader()->id() == $this->id()) return false; 		// can not vote on self
		if($upload->blocked()) return false;						// can not vote if blocked
		if($this->alreadyVoted($upload)) return false;	// can not vote again
		if(!$this->canComment($upload)) return false;		// can not vote if can not comment
		if(!$upload->file()) return false;							// can not vote if there is no file (topics, expired uploads)
		return true;
	}
	
	public function canComment($upload) {
		if(is_numeric($upload)) $upload = new Upload($upload);
		if(!$upload || !$upload->exists()) throw new Exception("invalid upload", 404);
		
		// no need to memoize, this method is not responsible for any new data
		if($this->status() != "admin" && $upload->blocked()) return false;	// non-admin blocked
		if($upload->id() == 211604 && me()->status() != "admin") return false; // changeblog
		if($upload->id() == 197431 && me()->status() != "admin") return false; // rhi-rhi ipod
		return true;
	}

	// API support
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

?>
