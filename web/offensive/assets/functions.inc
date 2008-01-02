<?
function linkUrls( $input ) {

	// replace urls with 'themaxx.com' in them with the current server name.
	$p = "/(http[s]*:\/\/[w]*\.*)themaxx\.com/i";
	$r = "\\1" . $_SERVER['SERVER_NAME'];
	$domainSwapped = preg_replace( $p, $r, $input );

	$pattern = "/(http[s]*:\/\/[^\s<>]+)/i";
	$replacement = "<a href=\"\\1\">\\1</a>";
	return preg_replace( $pattern, $replacement, $domainSwapped );
}

function sqlEscape($string) {
	if(ini_get("magic_quotes_gpc")) {
		$string = str_replace( "\\'", "'", $string );
		$string = str_replace( "\\\"", "\"", $string );
	}
	return mysql_real_escape_string($string);
}

function htmlEscape($string) {
	return str_replace(array("<", ">", "\""), 
	                   array("&lt;", "&gt;", "&quot;"),
	                   preg_replace("/&(?!#)/", 
	                                "&amp;", 
	                                $string));
}

?>
