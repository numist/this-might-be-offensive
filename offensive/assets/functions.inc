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
