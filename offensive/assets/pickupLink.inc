<?

class PickupLink {

	public static function content($c=null) {
		static $_content = null;
		
		if($c === null && $_content === null && array_key_exists("c", $_REQUEST)) {
			$_content = $_REQUEST["c"];
		} else if($c) {
			$_content = $c;
		}
		if($_content === null) {
			trigger_error("cannot determine content type", E_USER_ERROR);
		}
		
		return $_content;
	}
	
	private static function keys() {
		$ret = array();
		switch(PickupLink::content()) {
			case "yearbook":
			case "avatar":
				$ret["pref"] = "ypickup";
				$ret["cookie"] = me()->id()."lastavatar";
				break;
			case "audio":
				$ret["pref"] = "apickup";
				$ret["cookie"] = me()->id()."lasttrack";
				break;
			case "main":
			case "thumbs":
			case "image":
				$ret["pref"] = "ipickup";
				$ret["cookie"] = me()->id()."lastpic";
				break;
			case "discussions":
			case "topic":
				$ret["pref"] = "tpickup";
				$ret["cookie"] = me()->id()."lasttopic";
				break;
		}
		return $ret;
	}
	
	public static function get() {
		$ret = array();
	
		$keys = PickupLink::keys();
		if(count($keys) < 2) return $ret;
	
		// get db pickuplink
		if(me()->getPref($keys["pref"]) != null) {
			$ret["db"] = me()->getPref($keys["pref"]);
		}
		
		// get cookie pickuplink
		if(array_key_exists($keys["cookie"], $_COOKIE) && is_intger($_COOKIE[$keys["cookie"]])) {
			$ret["cookie"] = $_COOKIE[$keys["cookie"]];
		}
		
		return $ret;
	}
	
	// returns whether a (or both) pickuplink was updated
	public static function update($upload) {
		$ret = false;
	
		if(!is_object($upload)) {
			$upload = new Upload($upload);
		}
		
		$keys = PickupLink::keys();
		if(count($keys) < 2) {
			return false;
		}
		// XXX: should check type against content for key congruency?
		
		// update the pickup cookie
	 	if(!array_key_exists($keys["cookie"], $_COOKIE) ||
	     !is_intger($_COOKIE[$keys["cookie"]]) ||
	     $_COOKIE[$keys["cookie"]] < $upload->id()) {
	 		setcookie($keys["cookie"], $upload->id(), time() + 3600*24*365*10, "/offensive/");
			$ret = true;
		}
		
		// update the pickup db entry
		if(me()->getPref($keys["pref"]) == false || me()->getPref($keys["pref"]) < $upload->id()) {
			me()->setPref($keys["pref"], $upload->id());
			$ret = true;
		}
		
		return $ret;
	}
	
	public static function emit() {
		$links = PickupLink::get();
		$dbpic = array_key_exists("db", $links) ? $links["db"] : false;
		$cookiepic = array_key_exists("cookie", $links) ? $links["cookie"] : false;
		
		// output correct pickuplink
		if($dbpic == $cookiepic && $dbpic !== false) {
			?><div id="pickuplink" fileid="<?= $dbpic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">click here to pick up where you left off</a></b></div><?
		} else if ($dbpic && $cookiepic){
			?><div id="pickuplink"><b>pick up where you left off 
				(<a href="pages/pic.php?id=<?= $cookiepic ?>" fileid="<?= $cookiepic ?>" id="pickUp">on this computer</a> | 
				<a href="pages/pic.php?id=<?= $dbpic ?>" fileid="<?= $dbpic ?>" id="pickUp">on this account</a>)</b></div><?
		} else if ($cookiepic !== false) {
			trigger_error("this shouldn't ever happen anymore...", E_USER_NOTICE);
			?><div id="pickuplink" fileid="<?= $cookiepic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">pick up where you left off on this computer</a></b></div><?
		} else if ($dbpic !== false) {
			?><div id="pickuplink" fileid="<?= $dbpic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">pick up where you left off on this account</a></b></div><?
		}
	}
}
?>