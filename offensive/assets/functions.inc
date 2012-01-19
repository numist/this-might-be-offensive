<?

// XXX: this public function may be up for deprecation
function linkUrls( $input ) {
	trigger_error("deprecated: use Comment::HTMLcomment() from now on.", E_USER_ERROR);
}

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

function insertChange($command, $id, $data = null) {
	global $redis;
	$count = $redis->incr("changelog:counter");
	$time = microtime(true);
	$insert = $time . ':' . $count . ':' . $command . ':' . $id;
	$insert .= $data != null ? ':'.$data : '';
	$redis->lpush("changelog", $insert);
	$redis->ltrim("changelog", 0, 100);
}

function currentChange() {
	global $redis;
	return $redis->get("changelog:counter");
}

// XXX: fix: missing blackbar
function box($message, $title="hmm:") {
	?>
		<div class="heading"><?= $title ?></div>
		<div class="bluebox">
			<div style="text-align:center">
				<?= $message ?>
			</div>
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