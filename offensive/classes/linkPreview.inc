<?php

/*
 * previews for links in comments, shown on hover, on tapping the button
 * before the link, or inline for everything (see theme.css).
 *
 * each provider maps urls it recognizes to a canonical url and an oEmbed
 * endpoint.  previews are only ever read from redis while rendering; a miss
 * renders a plain link and queues the fetch for after the response has been
 * sent, so a page never waits on a third party.
 *
 * the server only ever requests a provider's own endpoint with a url it built
 * itself, never a url a user typed, so this can't be used to make tmbo fetch
 * arbitrary hosts.
 *
 * to add a provider, add an entry to providers(): "pattern" captures an id
 * from the url, "url" turns that id back into the canonical url (%s is the
 * id), and "oembed" is the provider's oEmbed endpoint.
 */
class LinkPreview {
	const TTL = 2592000;         // 30 days
	const MISS_TTL = 86400;      // retry deleted/private/broken links daily
	const FETCH_TIMEOUT = 2;
	const MAX_FETCHES = 10;      // per request

	private static $queue = array();

	private static function providers() {
		return array(
			"youtube" => array(
				"pattern" => '/^https?:\/\/(?:(?:www|m|music)\.)?(?:youtube\.com\/(?:watch\?(?:.*&)?v=|shorts\/|embed\/|live\/)|youtu\.be\/)([A-Za-z0-9_-]{11})(?![A-Za-z0-9_-])/i',
				"url" => "https://www.youtube.com/watch?v=%s",
				"oembed" => "https://www.youtube.com/oembed?format=json&url=",
			),
		);
	}

	// returns array(provider, id) for a url some provider handles, or null.
	private static function identify($url) {
		foreach(self::providers() as $name => $provider) {
			if(preg_match($provider["pattern"], $url, $matches)) {
				return array($name, $matches[1]);
			}
		}
		return null;
	}

	private static function key($name, $id) {
		return "linkpreview:$name:$id";
	}

	/*
	 * the cached preview for $url (unescaped) as an array with "title",
	 * "provider" and maybe "thumbnail", or null if there isn't one (yet).
	 */
	public static function get($url) {
		global $redis;

		$match = self::identify($url);
		if(!$match || !$redis) return null;

		$key = self::key($match[0], $match[1]);
		try {
			$cached = $redis->get($key);
		} catch(Exception $e) {
			trigger_error("link preview cache: ".$e->getMessage(), E_USER_NOTICE);
			return null;
		}

		if($cached === null) {
			self::enqueue($key, $match);
			return null;
		}

		$preview = json_decode($cached, true);
		return (is_array($preview) && isset($preview["title"])) ? $preview : null;
	}

	/*
	 * $link (the <a> for $url, unescaped) with its preview: a button before it
	 * that opens the preview as a popover, and the preview after it.  $id must
	 * be unique on the page.  $link alone if there is no preview.
	 */
	public static function html($url, $id, $link) {
		$preview = self::get($url);
		if(!$preview) return $link;

		$img = "";
		if(isset($preview["thumbnail"])) {
			$img = '<img src="'.htmlEscape($preview["thumbnail"]).'" alt="" loading="lazy" referrerpolicy="no-referrer" />';
		}
		return '<button type="button" popovertarget="'.$id.'" aria-label="preview"></button>'.$link.
		       '<span id="'.$id.'" popover>'.$img."<span>".htmlEscape($preview["title"])."</span><small>".htmlEscape($preview["provider"])."</small></span>";
	}

	private static function enqueue($key, $match) {
		if(empty(self::$queue)) {
			register_shutdown_function(array("LinkPreview", "fetchQueued"));
		}
		self::$queue[$key] = $match;
	}

	public static function fetchQueued() {
		global $redis;

		// only fetch once the visitor has their page.  without fpm there's no
		// way to do that, so leave the links plain rather than slow the page.
		if(!function_exists("fastcgi_finish_request")) return;
		// fastcgi_finish_request() doesn't release the session lock, and this
		// visitor's next request would wait on it.
		if(session_status() == PHP_SESSION_ACTIVE) session_write_close();
		fastcgi_finish_request();

		$providers = self::providers();
		foreach(array_slice(self::$queue, 0, self::MAX_FETCHES, true) as $key => $match) {
			list($name, $id) = $match;
			try {
				// another request is already fetching this one
				if(!$redis->set("$key:lock", 1, "EX", 60, "NX")) continue;

				$preview = self::fetch($name, $providers[$name], $id);
				$redis->setex($key, $preview ? self::TTL : self::MISS_TTL,
				              json_encode($preview ? $preview : new stdClass()));
			} catch(Exception $e) {
				trigger_error("link preview cache: ".$e->getMessage(), E_USER_NOTICE);
				return;
			}
		}
	}

	private static function fetch($name, $provider, $id) {
		$url = sprintf($provider["url"], $id);
		$context = stream_context_create(array("http" => array(
			"timeout" => self::FETCH_TIMEOUT,
			"ignore_errors" => true,
			"header" => "Accept: application/json\r\n",
		)));
		$body = @file_get_contents($provider["oembed"].rawurlencode($url), false, $context);
		// $http_response_header holds every response along any redirects;
		// the status line we care about is the last one.
		$status = array();
		if(isset($http_response_header)) {
			$status = preg_grep('/^HTTP\//', $http_response_header);
		}
		if($body === false || !preg_match('/^HTTP\/\S+ 200\b/', (string)end($status))) {
			return null;
		}

		$data = json_decode($body, true);
		if(!is_array($data) || !isset($data["title"]) || !is_string($data["title"])) {
			return null;
		}

		$preview = array(
			"title" => mb_substr($data["title"], 0, 200),
			"provider" => isset($data["provider_name"]) && is_string($data["provider_name"]) ?
			              $data["provider_name"] : $name,
		);
		if(isset($data["thumbnail_url"]) && is_string($data["thumbnail_url"]) &&
		   preg_match('/^https:\/\/[^\s"\'<>]+$/', $data["thumbnail_url"])) {
			$preview["thumbnail"] = $data["thumbnail_url"];
		}
		return $preview;
	}
}

?>