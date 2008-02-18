<?php

/* make TMBO read-only */
$readonly = false;

/* all pages require an active session eventually */

session_start();

/*  _                   _
 * | | ___   __ _  __ _(_)_ __   __ _
 * | |/ _ \ / _` |/ _` | | '_ \ / _` |
 * | | (_) | (_| | (_| | | | | | (_| |
 * |_|\___/ \__, |\__, |_|_| |_|\__, |
 *          |___/ |___/         |___/
 */

// if we are not an admin, buffer output until the end in case of fatal errors.
if(!array_key_exists("status", $_SESSION) || $_SESSION['status'] != "admin") {
        ob_start(null, 0);
}

// in tmbo, the include path should be a singleton path to the webroot.
if(false !== strpos(get_include_path(), PATH_SEPARATOR)) {
        trigger_error("path incorrectly set for tmbo environment (".
	              get_include_path().")", E_USER_ERROR);
}

function tmbo_error_handler($errno, $errstr, $errfile, $errline, $errcontext) {
        $errortype = array (
                E_ERROR              => 'Error',
                E_WARNING            => 'Warning',
                E_PARSE              => 'Parsing Error',
                E_NOTICE             => 'Notice',
                E_CORE_ERROR         => 'Core Error',
                E_CORE_WARNING       => 'Core Warning',
                E_COMPILE_ERROR      => 'Compile Error',
                E_COMPILE_WARNING    => 'Compile Warning',
                E_USER_ERROR         => 'User Error',
                E_USER_WARNING       => 'User Warning',
                E_USER_NOTICE        => 'User Notice',
                E_STRICT             => 'Runtime Notice',
                E_RECOVERABLE_ERROR  => 'Catchable Fatal Error'
                );
        $critical = E_ERROR | E_WARNING | E_PARSE | E_CORE_ERROR | E_CORE_WARNING | 
                    E_COMPILE_ERROR | E_COMPILE_WARNING | E_USER_ERROR | E_USER_WARNING;


        $shortfile = (strlen($errfile) > 25 ? 
	             "...".substr($errfile, -22) : $errfile);

        if(isset($_SESSION['status']) && $_SESSION['status'] == "admin" && $errno & $critical) {
                echo "<br />\n".
                     "<b>".$errortype[$errno]."</b>: $errstr in ".
		     "<b>$shortfile</b> on line <b>$errline</b><br />";
        }

        error_log("$shortfile:$errline:[".$errortype[$errno]."] ".$errstr);

        // handle fatal errors by sending users to kaboom, don't bug admins
        if($errno & (E_ERROR | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR)) {
                if(!array_key_exists("status", $_SESSION) ||
                   $_SESSION['status'] != "admin") {
		   	ob_end_clean();
                        header("Location: /offensive/index.outoforder.php");
		}
                exit;
        }
}
set_error_handler("tmbo_error_handler");

/* timezone error fix applies to all pages */
date_default_timezone_set("America/Los_Angeles");

/* debugging information gathering */
$queries = 0;
$querytime = 0;

function time_start(&$time) {
	$time = microtime(true);
}

// time break will reset the clock
function time_break(&$time) {
	$elapsed = microtime(true) - $time;
	time_start($time);
	return $elapsed;
}

// time end will not
function time_end($time) {
	return microtime(true) - $time;
}

function tmbo_query($sql) {
	global $link, $queries, $querytime, $readonly;
	
	if(!$link) {
		trigger_error("MySQL link not pre-existent or invalid", E_USER_WARNING);
		require_once('admin/mysqlConnectionInfo.inc');
		$link = openDbConnection();
	}

	$trace = debug_backtrace();
	$trace = $trace[0];

	if($readonly && 
	   ((strpos(strtolower(trim($sql)), "select") !== 0 && 
	     strpos(strtolower(trim($sql)), "show") !== 0) || 
	    strpos($sql, ";") !== false)) {
		return;
	}

	time_start($time);
	
	$result = mysql_query($sql, $link) or 
		tmbo_error_handler(E_USER_ERROR, mysql_error()." query '$sql'", 
		                   $trace['file'], $trace['line'], null);
	$time = time_end($time);
	$querytime += $time;

	if($time > 2) {
		$trace = debug_backtrace();
		$trace = $trace[0];
		tmbo_error_handler(E_USER_NOTICE, "query took ".number_format($time, 0).
		                   "s: \"$sql\"", $trace['file'], $trace['line'], null);
	}

	$queries++;
	return $result;
}

/* accurate timekeeping */
function record_hit() {
	if(array_key_exists("userid", $_SESSION)) {
		global $link;
	
		if(!isset($link) || !$link) {
			require_once("admin/mysqlConnectionInfo.inc");
			$link = openDbConnection();
		}
		
		tmbo_query("UPDATE users SET timestamp=now() WHERE userid=".$_SESSION['userid']." LIMIT 1");
	}
}

function mustLogIn() {
	if(array_key_exists("userid", $_SESSION) && is_numeric($_SESSION['userid'])) {
		return;
	}

	session_unset();

	ob_end_clean();
	header("Location: /offensive/logn.php?redirect=".urlencode($_SERVER['REQUEST_URI']));
	exit;
}

function byte_format($size, $digits=2) {	
	if($size > 1073741824) {
		return number_format(($size / 1073741824), $digits)."GB";
	} else if ($size > 1048576) {
		return number_format(($size / 1048576), $digits)."MB";
	} else if ($size > 1024) {
		return number_format(($size / 1024), $digits)."KB";
	} else {
		return $size."B";
	}
}

function data_url($file, $mime) 
{
  $contents = file_get_contents($file) or trigger_error("couldn't find $file from ".getcwd(), E_USER_ERROR);
  $base64   = base64_encode($contents); 
  return ('data:' . $mime . ';base64,' . $base64);
}

function print_url($filepath, $webpath, $mime=null) {
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
			return $webpath;
		}
	}
	
	echo data_url($filepath, $mime);
}

?>
