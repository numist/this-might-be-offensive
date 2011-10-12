<?php

// circular dependencies!
require_once("offensive/assets/classes.inc");

/* make TMBO read-only */
$readonly = false;

/* if tmbo is down, and you're working on it, set this variable to true.
 * this will display a fixing page to all users, and put a notice at the
 * top of all index pages to notify admins, helping to  prevent any
 * further possible damage.
 */
$fixing = false;
// now, stay calm, let's do this thing.

/* if tmbo is going down for an upgrade, set this to true and get to work.
 * this will display an upgrading page to all users, and will put a notice
 * at the top of all index pages notifying admins that the site is being
 * worked on.
 */
$upgrading = false;

/*****************************************************************************/

// force ssl.
if(!ssl()) {
	// TODO: if a token was present in the URI, it should be invalidated.
	header("Location: https://".$_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"], 301);
	exit;
}

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

function var_info($var) {
	ob_start();
	var_dump($var);
	$str=ob_get_contents();
	ob_end_clean();
	return $str;
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

        error_log("$shortfile:$errline:[".$errortype[$errno]."] ".$errstr);

		if(!($errno & error_reporting())) return;

        if(isset($_SESSION['status']) && $_SESSION['status'] == "admin" && $errno & $critical) {
                echo "<br />\n".
                     "<b>".$errortype[$errno]."</b>: ".nl2br($errstr)." in ".
		     "<b>$shortfile</b> on line <b>$errline</b><br />";
        }

        // handle fatal errors by sending users to the kaboom page, don't redirect admins
        if($errno & (E_ERROR | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR)) {
                if((!array_key_exists("status", $_SESSION) ||
				   $_SESSION['status'] != "admin") && ob_get_level() > 0) {
					ob_end_clean();
					require("offensive/index.outoforder.php");
				} else if (array_key_exists("status", $_SESSION) &&
					       $_SESSION['status'] == "admin") {
					ob_start();
					var_dump(debug_backtrace());
					$trace = nl2br(ob_get_contents());
					ob_end_clean();
					echo $trace;
				}
                exit;
        }
}
set_error_handler("tmbo_error_handler");
error_reporting(E_STRICT | E_ALL);

/* timezone error fix applies to all pages */
date_default_timezone_set("America/Los_Angeles");

/* debugging information gathering */
$queries = array();
$querytime = 0;

function is_intger($arg) {
	return (is_numeric($arg) && floor($arg) == ceil($arg));
}

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

function tmbo_query($sql, $timeout=2) {
	global $link, $queries, $querytime, $readonly;
	
	if(!$link) {
		trigger_error("MySQL link not pre-existent or invalid", E_USER_ERROR);
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

	if($time > $timeout && $timeout != 0) {
		$trace = debug_backtrace();
		$trace = $trace[0];
		
		$shortfile = (strlen($trace['file']) > 25 ? 
		         "...".substr($trace['file'], -22) : $trace['file']);
        
		error_log("$shortfile:{$trace['file']}: query took ".number_format($time, 0).
			                   "s: \"$sql\"");
	}

	$queries[] = array($sql, $time);
	return $result;
}

/* accurate timekeeping */
function record_hit() {
	if(login()) {
		tmbo_query("UPDATE users SET timestamp=now() WHERE userid=".$_SESSION['userid']." LIMIT 1");
	}
}

function byte_format($size, $digits=0) {	
	if($size > (1073741824 * 4)) {
		return number_format(($size / 1073741824), $digits)."GB";
	} else if ($size > (1048576 * 4)) {
		return number_format(($size / 1048576), $digits)."MB";
	} else if ($size > (1024 * 4)) {
		return number_format(($size / 1024), $digits)."KB";
	} else {
		return $size."B";
	}
}

function me() {
	global $me, $login_called;
	// get the User object for the logged-in user
	if(is_object($me)) return $me;
	
	if(array_key_exists('userid', $_SESSION)) {
		trigger_error("was able to get user from session—".
		              "should have called login() already!",
		              E_USER_WARNING);
		$me = new User($_SESSION['userid']);
		return $me;
	}
	
	if(!$login_called) {
		trigger_error("call login() before me()", E_USER_WARNING);
	}
	return null;
}

// logn.inc is needed for all authentication
require_once("offensive/assets/logn.inc");

?>
