<?php

/*
 * previews for links in comments, shown on hover, on tapping the button
 * before the link, or inline for everything (see theme.css).
 *
 * each provider maps urls it recognizes to a canonical url and an oEmbed
 * endpoint.  previews are only ever read from redis while rendering; a miss
 * renders a plain link and queues the fetch for after the response has been
 * sent, so a page never waits on a third party.  previews don't expire: one
 * older than REFRESH_AGE is still shown, and refreshed the same way, so a
 * link that has had a preview keeps it unless the provider says it's gone.
 * admin/linkPreviewBackfill.php fills the cache for links nobody has viewed.
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
	const REFRESH_AGE = 2592000; // 30 days
	const MISS_TTL = 86400;      // the provider says no (deleted, private): retry daily
	const RETRY_TTL = 900;       // timeouts, rate limits, outages: retry soon
	const FETCH_TIMEOUT = 2;
	const MAX_FETCHES = 50;      // per request, fetched all at once
	const MAX_SERIAL_FETCHES = 10; // per request, one at a time without curl

	private static $queue = array();

	private static function providers() {
		return array(
			"youtube" => array(
				"pattern" => '/^https?:\/\/(?:(?:www|m|music)\.)?(?:youtube\.com\/(?:watch\?(?:.*&)?v=|shorts\/|embed\/|live\/|v\/)|youtu\.be\/)([A-Za-z0-9_-]{11})(?![A-Za-z0-9_-])/i',
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
	 * "provider", "fetched" and maybe "thumbnail", or null if there isn't one
	 * (yet).  a missing or stale preview is queued to be fetched.
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
			self::enqueue($key, $match, null);
			return null;
		}

		$preview = json_decode($cached, true);
		if(!is_array($preview) || !isset($preview["title"])) return null;
		// previews cached before they had a fetch time count as stale
		if(!isset($preview["fetched"]) || $preview["fetched"] < time() - self::REFRESH_AGE) {
			self::enqueue($key, $match, $preview);
		}
		return $preview;
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

	private static function enqueue($key, $match, $preview) {
		if(empty(self::$queue)) {
			register_shutdown_function(array("LinkPreview", "fetchQueued"));
		}
		self::$queue[$key] = array("match" => $match, "preview" => $preview);
	}

	public static function fetchQueued() {
		// only fetch once the visitor has their page.  without fpm there's no
		// way to do that, so leave the links plain rather than slow the page.
		if(!function_exists("fastcgi_finish_request")) return;
		// fastcgi_finish_request() doesn't release the session lock, and this
		// visitor's next request would wait on it.
		if(session_status() == PHP_SESSION_ACTIVE) session_write_close();
		fastcgi_finish_request();

		self::fetch(function_exists("curl_multi_init") ? self::MAX_FETCHES : self::MAX_SERIAL_FETCHES);
	}

	// how many links are waiting to be fetched
	public static function queued() {
		return count(self::$queue);
	}

	/*
	 * fetches up to $max queued previews, missing ones before stale ones, and
	 * caches what comes back.  returns the response status of each fetch
	 * (0 for no response) by cache key; links another request is already
	 * fetching are skipped and left out.
	 */
	public static function fetch($max) {
		global $redis;

		$providers = self::providers();
		$missing = array_filter(self::$queue, function($q) { return $q["preview"] === null; });
		$batch = array_slice($missing + self::$queue, 0, $max, true);
		self::$queue = array_diff_key(self::$queue, $batch);

		$statuses = array();
		try {
			$urls = array();
			foreach($batch as $key => $q) {
				// another request is already fetching this one
				if(!$redis->set("$key:lock", 1, "EX", 60, "NX")) continue;

				list($name, $id) = $q["match"];
				$urls[$key] = $providers[$name]["oembed"].rawurlencode(sprintf($providers[$name]["url"], $id));
			}

			$responses = function_exists("curl_multi_init") ? self::requestAll($urls) : array_map(array("LinkPreview", "request"), $urls);
			foreach($responses as $key => $response) {
				list($status, $body) = $response;
				$statuses[$key] = $status;
				$old = $batch[$key]["preview"];
				$preview = $status == 200 ? self::parse($batch[$key]["match"][0], $body) : null;
				// a bad id, a private video, or a deleted one won't change soon.
				// anything else (no answer, 429, 5xx) might.
				$gone = $status == 200 || in_array($status, array(400, 401, 404));
				if($preview) {
					$preview["fetched"] = time();
					$redis->set($key, json_encode($preview));
				} else if($gone) {
					$redis->setex($key, self::MISS_TTL, json_encode(new stdClass()));
				} else if($old) {
					// keep showing it, and try again in RETRY_TTL
					$old["fetched"] = time() - self::REFRESH_AGE + self::RETRY_TTL;
					$redis->set($key, json_encode($old));
				} else {
					$redis->setex($key, self::RETRY_TTL, json_encode(new stdClass()));
				}
				$redis->del("$key:lock");
			}
		} catch(Exception $e) {
			trigger_error("link preview cache: ".$e->getMessage(), E_USER_NOTICE);
		}
		return $statuses;
	}

	/*
	 * GETs every url at once.  returns array(status, body) by the same keys;
	 * status is 0 if there was no response in time.  handles are freed when
	 * they go out of scope (curl_close() does nothing since php 8).
	 */
	private static function requestAll($urls) {
		$multi = curl_multi_init();
		$handles = array();
		foreach($urls as $key => $url) {
			$handle = curl_init($url);
			curl_setopt_array($handle, array(
				CURLOPT_RETURNTRANSFER => true,
				CURLOPT_FOLLOWLOCATION => true,
				CURLOPT_MAXREDIRS => 3,
				CURLOPT_PROTOCOLS => CURLPROTO_HTTPS,
				CURLOPT_REDIR_PROTOCOLS => CURLPROTO_HTTPS,
				CURLOPT_TIMEOUT => self::FETCH_TIMEOUT,
				CURLOPT_HTTPHEADER => array("Accept: application/json"),
			));
			curl_multi_add_handle($multi, $handle);
			$handles[$key] = $handle;
		}

		do {
			$status = curl_multi_exec($multi, $running);
			if($running) curl_multi_select($multi);
		} while($running && $status == CURLM_OK);

		$responses = array();
		foreach($handles as $key => $handle) {
			// the response code is 0 when there was no response
			$responses[$key] = array(curl_getinfo($handle, CURLINFO_RESPONSE_CODE),
			                         (string)curl_multi_getcontent($handle));
			curl_multi_remove_handle($multi, $handle);
		}
		return $responses;
	}

	// GETs $url without curl.  returns array(status, body) like requestAll().
	private static function request($url) {
		$context = stream_context_create(array("http" => array(
			"timeout" => self::FETCH_TIMEOUT,
			"ignore_errors" => true,
			"header" => "Accept: application/json\r\n",
		)));
		$body = @file_get_contents($url, false, $context);
		// $http_response_header holds every response along any redirects;
		// the status line we care about is the last one.
		$status = array();
		if(isset($http_response_header)) {
			$status = preg_grep('/^HTTP\//', $http_response_header);
		}
		if($body === false || !preg_match('/^HTTP\/\S+ (\d{3})\b/', (string)end($status), $m)) {
			return array(0, "");
		}
		return array((int)$m[1], $body);
	}

	// the preview in an oEmbed response, or null if it doesn't have one.
	private static function parse($name, $body) {
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