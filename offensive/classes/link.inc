<?

class Link {
	static function setPref($key, $value) {
		require("offensive/data/preferences.inc");
		if(!in_array($key, $prefnames))
			trigger_error("$key is not a valid preference name!", E_USER_ERROR);
		
		return "/offensive/setPref.php?p=".$key."&v=".$value;
	}
	
	static function listFor($upload) {
		if(!is_object($upload)) {
			$upload = new Upload($upload);
		}
		
		switch($upload->type()) {
			case "topic":
				return Link::content("discussions");
			case "audio":
				return Link::content("audio");
			case "avatar":
				return Link::content("yearbook");
			default:
				trigger_error("unknown type ".$upload->type(), E_USER_WARNING);
				// intentional fall-through
			case "image":
				return Link::mainpage();
		}
	}
	
	static function mainpage() {
		return Link::content(me() ? me()->getPref("index") : "main");
	}
	
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

?>