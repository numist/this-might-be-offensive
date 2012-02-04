<?

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

?>