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
	if(loggedin()) {
		tmbo_query("UPDATE users SET timestamp=now() WHERE userid=".$_SESSION['userid']." LIMIT 1");
	}
}

function loggedin($lite=true) {
	if(!array_key_exists('userid', $_SESSION) || !is_intger($_SESSION['userid'])) return false;
	
	/* lite checks do not make a round trip to the database.  this is good enough for code
	 * sanity checks, but not good enough for authorization checks that may want to check
	 * the user's status against the server.
	 * for instance, in the event a user gets banned, the lite sanity check will pass (code
	 * is ok to execute -- it will not break), but a heavy check will notice that the user
	 * has been banned and fail (user is not ok to continue -- user has been banned)
	 */
	if($lite) {
		return true;
	}
	
	$result = tmbo_query("SELECT * FROM users WHERE userid = ".$_SESSION['userid']);
	if(mysql_num_rows($result) == 0) return false;
	$row = mysql_fetch_assoc($result);
	
	if($row['account_status'] == 'locked' || $row['account_status'] == 'awaiting activation') return false;
	
	// initialize a User object so we don't have to go to the DB again later for it
	$throwaway = new User($row);
	
	if($_SESSION['username'] != $row['username']) $_SESSION['username'] = $row['username'];
	if($_SESSION['status'] != $row['account_status']) $_SESSION['status'] = $row['account_status'];
	
	return true;
}

/* 
 * this function ONLY returns if the user is logged in.  
 * if the user is not logged in (or fails to authenticate, in the case of HTTP
 * auth), mustLogIn will exit the script.
 */
// logn.inc is needed for the call to login().
require_once("offensive/assets/logn.inc");
function mustLogIn($type="logn") {
	// login checks are authorization checks, so a heavy loggedin() request is required.
	if(loggedin(false)) {
		return;
	}
	// check for rememberme cookie
	if(login()) {
		return;
	}
	
	if(!isset($_SERVER["HTTPS"]) || $_SERVER["HTTPS"] != "on") {
		$server = "http://".$_SERVER["SERVER_NAME"];
	} else {
		$server = "https://".$_SERVER["SERVER_NAME"];
	}
	$redirect = (isset($_REQUEST['redirect']) ? $_REQUEST['redirect'] : $server.urlencode($_SERVER['REQUEST_URI']));

	ob_end_clean();
	
	session_unset();
	
	if($type == "http") {
		if (isset($_SERVER['PHP_AUTH_USER']) && strlen($_SERVER['PHP_AUTH_USER']) > 0) {
			require_once('offensive/assets/logn.inc');
			global $login_message;
			
			if(login($_SERVER['PHP_AUTH_USER'], $_SERVER['PHP_AUTH_PW']) === true) return;

			header('HTTP/1.0 401 Not Found');
	    	echo $login_message;
			exit;
		}
		// logout business, 403.
		header('WWW-Authenticate: Basic realm="[this might be offensive]"');
	    header('HTTP/1.0 401 Unauthorized');
	    echo 'anon is not welcome here.';
	    exit;
	} else {
		header("Location: https://".$_SERVER["SERVER_NAME"]."/offensive/logn.php?redirect=".$redirect);
		exit;
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
	global $me;
	// get the User object for the logged-in user
	if(is_object($me)) return $me;
	
	if(array_key_exists('userid', $_SESSION)) {
		$me = new User($_SESSION['userid']);
		return $me;
	}
	
	trigger_error("user not discernable", E_USER_WARNING);
	return null;
}

?>
