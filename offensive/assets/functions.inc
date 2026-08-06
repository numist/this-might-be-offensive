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

/* fetch a secret out of admin/.config (which is not in source control).
 *
 * these used to be string literals in the source, which meant anyone reading
 * the public repository could forge a remember cookie for any account.
 * secrets live in the config file now and must never come back.
 *
 * this is a function rather than a global assignment on purpose: api.php and
 * activate.php both include activationFunctions.inc *before* they include
 * admin/mysqlConnectionInfo.inc, so $config does not exist yet at the time
 * those files are parsed.  resolve it lazily, at the point of use.
 *
 * a missing or obviously-too-short secret is fatal.  falling back to a
 * default would quietly reintroduce the bug this replaced.
 */
function tmbo_secret($key) {
	global $config;

	if(!isset($config) || !is_array($config) ||
	   !array_key_exists($key, $config) || strlen($config[$key]) < 32) {
		trigger_error("missing or too-short secret \"$key\" in admin/.config", E_USER_ERROR);
	}

	return $config[$key];
}

/* generate $bytes cryptographically strong random bytes, hex encoded.
 * this runs on php 5.5, so random_bytes() is not available.  every source
 * below is a CSPRNG: there is deliberately no mt_rand()/uniqid() fallback,
 * because predictable output here silently recreates the forgeable cookie
 * bug this function exists to fix.  no entropy means no login, not a weak one.
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

	trigger_error("no source of cryptographic randomness available", E_USER_ERROR);
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
