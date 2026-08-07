<?

function sqlEscape($string) {
	if(ini_get("magic_quotes_gpc")) {
		$string = str_replace( "\\'", "'", $string );
		$string = str_replace( '\\"', '"', $string );
	}
	return mysql_real_escape_string($string);
}

function htmlEscape($string) {
	return str_replace(array("<", ">", '"', "'"), 
	                   array("&lt;", "&gt;", "&quot;", "&#39;"),
	                   preg_replace("/&(?!#)/", 
	                                "&amp;", 
	                                $string));
}

/* fetch a secret out of admin/.config, which is not in source control.
 *
 * resolves at the point of use rather than at parse time: core.inc includes
 * activationFunctions.inc before admin/mysqlConnectionInfo.inc, so $config does
 * not exist yet while this function's callers are being parsed.
 *
 * a missing or too-short secret is fatal, and there is deliberately no default.
 */
function tmbo_secret($key) {
	global $config;

	if(!isset($config) || !is_array($config) ||
	   !array_key_exists($key, $config) || strlen($config[$key]) < 32) {
		/* E_USER_ERROR only terminates by way of tmbo_error_handler, which is
		 * installed by header.inc and returns early if error_reporting is lowered.
		 * falling through would return null, which concatenates into a caller's
		 * key as "" -- the default this function promises not to have.
		 */
		trigger_error("missing or too-short secret \"$key\" in admin/.config", E_USER_ERROR);
		exit;
	}

	return $config[$key];
}

/* generate $bytes cryptographically strong random bytes, hex encoded.
 * php 5.5 has no random_bytes(), so the sources are tried in turn by hand.
 * every one is a CSPRNG, and there is deliberately no mt_rand()/uniqid()
 * fallback: callers use this for credentials, so failing outright is correct
 * and predictable output is not.
 */
function tmbo_random_hex($bytes) {
	if(function_exists("openssl_random_pseudo_bytes")) {
		$strong = false;
		$raw = openssl_random_pseudo_bytes($bytes, $strong);
		if($raw !== false && $strong) return bin2hex($raw);
	}

	if(function_exists("mcrypt_create_iv")) {
		$raw = @mcrypt_create_iv($bytes, MCRYPT_DEV_URANDOM);
		if($raw !== false && strlen($raw) == $bytes) return bin2hex($raw);
	}

	$fh = @fopen("/dev/urandom", "rb");
	if($fh !== false) {
		$raw = @fread($fh, $bytes);
		fclose($fh);
		if($raw !== false && strlen($raw) == $bytes) return bin2hex($raw);
	}

	// see tmbo_secret() on why this does not rely on E_USER_ERROR to terminate:
	// returning null here hands the caller an empty credential.
	trigger_error("no source of cryptographic randomness available", E_USER_ERROR);
	exit;
}

/* compare two strings in constant time.
 * php 5.6 has hash_equals() for this; we don't, so it's spelled out here.
 * comparing secrets with == leaks their contents through timing, and php's
 * == will also happily type-juggle two numeric-looking strings into equality.
 */
function tmbo_hash_equals($known, $given) {
	if(!is_string($known) || !is_string($given)) return false;

	$len = strlen($known);
	if($len !== strlen($given)) return false;

	$diff = 0;
	for($i = 0; $i < $len; $i++) {
		$diff |= ord($known[$i]) ^ ord($given[$i]);
	}
	return $diff === 0;
}

/* per-session value proving a request was built by one of our own pages.
 *
 * the session cookie answers "who is this", and the browser attaches it to any
 * request to us however that request was provoked -- which is why a form another
 * site submits on a user's behalf arrives authenticated.  this value answers a
 * different question, and nothing attaches it automatically: it has to be written
 * into the request, and a page on another origin cannot read it to do so.
 *
 * minted on demand so anonymous forms can carry one too.  survives
 * session_regenerate_id(), which keeps $_SESSION and changes only the id.
 */
function csrfToken() {
	if(!array_key_exists('csrf', $_SESSION)) {
		$_SESSION['csrf'] = tmbo_random_hex(16);
	}
	return $_SESSION['csrf'];
}

/* does this request carry the token of the session making it?
 * deliberately does not mint one: a session with no token yet cannot have issued
 * the form being submitted.
 */
function csrfValid() {
	return array_key_exists('csrf', $_SESSION) &&
	       array_key_exists('csrf', $_REQUEST) &&
	       tmbo_hash_equals($_SESSION['csrf'], $_REQUEST['csrf']);
}

function getFileExtension($filename) {
	preg_match( "/\.[^\.]+$/", $filename, $matches );
	if(is_array($matches) && isset($matches[0])) return $matches[0];
	return null;
}

function ensureDirExists( $path ) {
	$basePath = dirname(realpath($_SERVER['SCRIPT_FILENAME']));	
	$curPath = $basePath;
	foreach( explode( '/', $path ) as $dir ) {
		$curPath .= "/$dir";
		if( ! file_exists( $curPath ) ) {
			mkdir( $curPath ) or trigger_error("could not create directory path $curPath from ".getcwd(), E_USER_ERROR);
		}
	}
}

function publishChange($type, $channels, $data) {
	global $redis;
	$payload = json_encode(array(
		'type' => $type,
		'data' => $data
	));
	foreach($channels as $channel) {
	 	$redis->publish($channel, $payload);
	}
}

function box($message, $title="hmm:") {
	?>
	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading"><?= $title ?></div>
		<div class="bluebox">
			<div style="text-align:center">
				<?= $message ?>
			</div>
		</div>
		<div class="blackbar"></div>
	</div>
	<?
}

function array_top($arr) {
	return reset($arr);
}

function ssl() {
	return (isset($_SERVER["HTTPS"]) && $_SERVER["HTTPS"] == "on");
}

function id($id) {
	return $id;
}

?>
