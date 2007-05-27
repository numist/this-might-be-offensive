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
?>
