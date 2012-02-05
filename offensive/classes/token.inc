<?

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