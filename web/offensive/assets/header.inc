<?php

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
if(array_key_exists("status", $_SESSION) && $_SESSION['status'] != "admin") {
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

        if(isset($_SESSION['status']) && $_SESSION['status'] == "admin") {
                echo "<br />\n".
                     "<b>".$errortype[$errno]."</b>: $errstr in ".
		     "<b>$errfile</b> on line <b>$errline</b><br />";
        }

        $shortfile = (strlen($errfile) > 25 ? 
	             "...".substr($errfile, -22) : $errfile);

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
date_default_timezone_set("America/Chicago");

/* debugging information gathering */
$queries = 0;
$querytime = 0;

function time_start(&$time) {
	$time = microtime(true);
}

function time_break($time) {
	return microtime(true) - $time;
}
function time_end($time) {
	return time_break($time);
}

function tmbo_query($sql) {
	global $link, $queries, $querytime;

	time_start($time);
	$result = mysql_query($sql, $link) or 
		trigger_error(mysql_error()." query '$sql'", E_USER_ERROR);
	$querytime += time_end($time);
	$queries++;
	return $result;
}

/* accurate timekeeping */
if(array_key_exists("userid", $_SESSION)) {
	global $link;

	if(!isset($link) || !$link) {
		require_once("admin/mysqlConnectionInfo.inc");
		$link = openDbConnection();
	}
	
	tmbo_query("UPDATE users SET timestamp=now() WHERE userid=".$_SESSION['userid']." LIMIT 1");
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

?>
