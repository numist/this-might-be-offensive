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

function data_url($file, $mime) 
{
  $contents = file_get_contents($file) or trigger_error("couldn't find $file from ".getcwd(), E_USER_ERROR);
  $base64   = base64_encode($contents); 
  return ('data:' . $mime . ';base64,' . $base64);
}

function print_url($filepath, $webpath, $mime=null) {

	/*
	 * [rh 2008.04.07] short-circuiting this because i'm sick of not seeing thumbnails on the comments pages.
	 */
	echo $webpath;
	return;
	
	/* Internet Explorer does not support the data: URI scheme
	 * http://en.wikipedia.org/wiki/Data:_URI
	 */
	if(ini_get("browscap")) {
		$browser = get_browser(null, true);
		if($browser['browser'] == "IE") {
			echo $webpath;
			return;
		}
	} else {
		echo $webpath;
		return;
	}
	
	// figure out mimetype based on file type, if possible
	if($mime == null) {
		if(ereg("[jJ][pP][eE]?[gG]$", $filepath)) {
			$mime = "image/jpeg";
		} else if(ereg("[gG][iI][fF]$", $filepath)) {
			$mime = "image/gif";
		} else if(ereg("[pP][nN][gG]$", $filepath)) {
			$mime = "image/png";
		} else {
			trigger_error("misunderstood file type: $filepath", E_USER_WARNING);
			echo $webpath;
		}
	}
	
	echo data_url($filepath, $mime);
}

?>