<?



function linkUrls( $input ) {

	// replace urls with 'themaxx.com' in them with the current server name.
	$p = "/(http[s]*:\/\/[w]*\.*)themaxx\.com/i";
	$r = "\\1" . $_SERVER['SERVER_NAME'];
	$domainSwapped = preg_replace( $p, $r, $input );
	
	// to prevent weird navigation things, do the same for tmbo.org
	$p = "/(http[s]*:\/\/[w]*\.*)tmbo\.org/i";
	$r = "\\1" . $_SERVER['SERVER_NAME'];
	$domainSwapped = preg_replace( $p, $r, $domainSwapped );
	
	// aand thismight.be
	$p = "/(http[s]*:\/\/[w]*\.*)thismight\.be/i";
	$r = "\\1" . $_SERVER['SERVER_NAME'];
	$domainSwapped = preg_replace( $p, $r, $domainSwapped );
	
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
	return str_replace(array("<", ">", "\"", "'"), 
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

/*
 * helper functions for getting paths to files and thumbnails
 */

function basePath($timestamp, $type, $nopath=false) {
	if(!is_numeric($timestamp))
		$timestamp = strtotime($timestamp);
	$year = date( "Y", $timestamp );
	$month = date( "m", $timestamp );
	$day = date( "d", $timestamp );
	return ($nopath ? "" : get_include_path()."/")."offensive/uploads/$year/$month/$day/$type/";
}

// XXX: to be deprecated!
function getThumb($id, $filename, $timestamp, $type='image') {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if($type != 'image' && $type != 'avatar') return '';
	
	$retname = basePath($timestamp, $type)."thumbs/th$id".getFileExtension($filename);
	
	if(!file_exists($retname)) {
		// what about yesterday?  or tomorrow?
		// NOTE: if timestamp is a number, don't fall for the infinite recursion trap
		if(!is_numeric($timestamp)) {
			$timestamp = strtotime($timestamp);
			$yesterday = $timestamp - 86400;
			$tomorrow = $timestamp + 86400;
			if(thumbConform(getThumb($id, $filename, $yesterday, $type), $id, $filename, $timestamp, $type)) return $retname;
			if(thumbConform(getThumb($id, $filename, $tomorrow, $type), $id, $filename, $timestamp, $type)) return $retname;
		}
		
		
		$bigfile = getFile($id, $filename, $timestamp, $type);
		if($bigfile != '') {
			// man, I sure hope this doesn't happen often.

			$ext = getFileExtension($filename);
			$tmpfile = get_include_path()."/offensive/quarantine/$id".rand().$ext;
			copy($bigfile, $tmpfile);

			$cmd = "convert -resize 100x100 $tmpfile $tmpfile";
			$err = shell_exec($cmd);
			if(strlen($err > 0)) {
				trigger_error("ImageMagick failed: $err", E_USER_WARNING);
				return '';
			}
			ensureDirExists(dirname($retname));
			rename($tmpfile, $retname);
			
			trigger_error("CONFORM: resized $bigfile to $retname", E_USER_NOTICE);
			
			return $retname;
		}
		
		return '';
	}

	return $retname;
}

// XXX: to be deprecated!
function getThumbURL($id, $filename, $timestamp, $type='image') {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if(getFile($id, $filename, $timestamp, $type) != '' && getThumb($id, $filename, $timestamp, $type) != '')
		return "/".basePath($timestamp, $type, true)."thumbs/th$id".getFileExtension($filename);
	return "/offensive/graphics/previewNotAvailable.gif";
}

// XXX: to be deprecated!
function getFile($id, $filename, $timestamp, $type='image') {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if($type == 'topic') return '';
	
	$retname = basePath($timestamp, $type).$id."_".$filename;
	
	// ignore the man behind the curtain
	if(!file_exists($retname)) {		
		// what about yesterday?  or tomorrow?
		// NOTE: if timestamp is a number, don't fall for the infinite recursion trap
		if(!is_numeric($timestamp)) {
			$timestamp = strtotime($timestamp);
			$yesterday = $timestamp - 86400;
			$tomorrow = $timestamp + 86400;
			if(fileConform(getFile($id, $filename, $yesterday, $type), $id, $filename, $timestamp, $type)) return $retname;
			if(fileConform(getFile($id, $filename, $tomorrow, $type), $id, $filename, $timestamp, $type)) return $retname;
		}

		return '';
	}
	
	return $retname;
}

// XXX: to be deprecated!
function getFileURL($id, $filename, $timestamp, $type='image') {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if($type == 'topic') return '';
		
	// quick hack to make sure getFile gets a chance to run its frantic search function.
	if(getFile($id, $filename, $timestamp, $type) == '') return '';
	
	return "/".basePath($timestamp, $type, true).$id."_".rawurlencode($filename);
}

/* these two functions are the main players allowing files to be reorganized
 * on the fly without having to take the site down to restructure everything all at once.
 */
function fileConform($path, $id, $filename, $timestamp, $type) {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if($path == '' || !file_exists($path)) return false;
	$target = basePath($timestamp, $type).$id."_".$filename;
	ensureDirExists(basePath($timestamp, $type));
	
	// if timestamp is numeric, we want to move, not copy
	if(is_numeric($timestamp) && !rename($path, $target)) {
		trigger_error("rename($path, $target) failed", E_USER_WARNING);
	} else if(!link($path, $target)) {
		trigger_error("link($path, $target) failed", E_USER_WARNING);
	}
	
	trigger_error("CONFORM: moved $path to $target", E_USER_NOTICE);
	return true;
}

function thumbConform($path, $id, $filename, $timestamp, $type) {
	if($id == null || $filename == '' || $timestamp == "" || $type == "") return "";
	
	if($path == '' || !file_exists($path)) return false;
	$target = basePath($timestamp, $type)."thumbs/th$id".getFileExtension($filename);
	ensureDirExists(basePath($timestamp, $type)."thumbs/");
	
	// if timestamp is numeric, we want to move, not copy
	if(is_numeric($timestamp) && !rename($path, $target)) {
		trigger_error("rename($path, $target) failed", E_USER_WARNING);
	} else if(!link($path, $target)) {
		trigger_error("link($path, $target) failed", E_USER_WARNING);
	}
	
	trigger_error("CONFORM: moved $path to $target", E_USER_NOTICE);
	return true;
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

function hideImage($is_nsfw, $is_tmbo, $uploaderid) {
	return ( array_key_exists("prefs", $_SESSION) &&
	    is_array($_SESSION['prefs']) &&
	    (array_key_exists("hide nsfw", $_SESSION['prefs']) && 
	    $_SESSION['prefs']['hide nsfw'] == 1 && $is_nsfw == 1) || 
	    (array_key_exists("hide tmbo", $_SESSION['prefs']) &&
	    $_SESSION['prefs']['hide tmbo'] == 1 && $is_tmbo == 1) 
		|| ( in_array( $uploaderid, explode( ',', $_SESSION['prefs']['squelched'] ) ) ) );
}

// XXX: to be deprecated!
function commentLabel($comments, $good, $bad, $tmbo, $votes=true) {
	if(!is_numeric($comments)) $comments = 0;
	
	$retval = ($comments == 0 ? "no" : $comments)." comment".($comments == 1?"":"s");
	if(!$votes) return $retval;
	if(!is_numeric($good)) $good = 0;
	if(!is_numeric($bad)) $bad = 0;
	if(!is_numeric($tmbo)) $tmbo = 0;
	$retval .= " (+$good -$bad".($tmbo > 0?" x$tmbo":"").")";
	return $retval;
}

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

/* $id can be:
 *  * numeric fileid (causes a round-trip to the database)
 *  * array(filename, nsfw, tmbo, type) (numerically indexed)
 *  * array containing keys filename, nsfw, tmbo, type (like a row from the db)
 */
function htmlFilename($id, $showtype=false) {
	if(!is_numeric($id) && !is_array($id)) {
		trigger_error("htmlFilename: ".var_info($id)." is not a number nor array!", E_USER_WARNING);
		return false;
	} else if (is_array($id)) {
		if(isset($id[0])) {
			list($filename, $nsfw, $tmbo, $type) = $id;
		} else if(array_key_exists('filename', $id) && 
				array_key_exists('nsfw', $id) && 
				array_key_exists('tmbo', $id) && 
				array_key_exists('type', $id)) {
			$filename = $id['filename'];
			$nsfw = (int)$id['nsfw'];
			$tmbo = (int)$id['tmbo'];
			$type = $id['type'];
		} else {
			var_dump($id);
			trigger_error("could not make heads or tails of array", E_USER_ERROR);
		}
	} else {
		$sql = "SELECT filename, nsfw, tmbo, type FROM offensive_uploads WHERE id = $id";
		$result = tmbo_query($sql);
		if(mysql_num_rows($result) === 0) {
			return false;
		}
		list($filename, $nsfw, $tmbo, $type) = mysql_fetch_array($result);
	}
	$tags = "";
	if($tmbo != 0) {
		$tags = "<span style=\"color:#990000\">[tmbo]</span>$tags";
	}
	if($nsfw != 0) {
		$tags = "<span style=\"color:#990000\">[nsfw]</span>$tags";
	}
	if($showtype) {
		$tags = "$type: $tags";
	}
	
	$filename = strlen($tags) > 0 ? "$tags $filename" : $filename;
	
	return htmlEscape($filename);
}

?>