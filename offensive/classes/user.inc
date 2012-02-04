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
			trigger_error("target must be a userid or User object", E_USER_ERROR);
		}
		
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
		
		return in_array($target, $this->data["squelched"]);
	}
	
	public function blocked($target) {
		if(is_object($target)) {
			$target = $target->id();
		}
		
		return $this->squelched($target) || in_array($target, $this->data["squelched_by"]);
	}

	// squelch a user
	public function squelch($target) {
		// sanity check
		if(me()->id() != $this->id() && me()->status() != "admin") {
			trigger_error(me()->username()." can not squelch on ".$this->username()."'s behalf!", E_USER_ERROR);
		}
		
		// is this redundant?
		if($this->squelched($target)) {
			return;
		}
		
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
		$this->data["squelched"][] = $target;
	}
	
	// unsquelch a user
	public function unsquelch($target) {
		// sanity check
		if(me()->id() != $this->id() && me()->status() != "admin") {
			trigger_error(me()->username()." can not squelch on ".$this->username()."'s behalf!", E_USER_ERROR);
		}
		
		// is this redundant?
		if(!$this->squelched($target)) {
			return;
		}
		
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
		global $redis;
		
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			trigger_error("$name is not a valid preference name!", E_USER_ERROR);
		
		$value = $redis->get($this->id().':'.$name);
		
		if($name == "sortorder_discussions" && $value == "") {
			return "date_desc";
		}
		
		return $value;
	}
	
	public function setPref($name, $value) {
		global $redis;
		require("offensive/data/preferences.inc");
		if(!in_array($name, $prefnames))
			trigger_error("$name is not a valid preference name!", E_USER_ERROR);
		if(!$this->exists())
			trigger_error("user(".$this->id().") does not exist", E_USER_ERROR);
		
		if(strlen($value) > 0) {
			$redis->set($this->id().':'.$name, $value);
		} else {
			$redis->del($this->id().':'.$name);
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

?>