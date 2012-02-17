<?php

set_include_path("../..");
// set up the normal TMBO environment
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
require_once("offensive/assets/argvalidation.inc");
require_once("offensive/assets/classes.inc");
require_once("offensive/assets/comments.inc");

mustLogIn(array("method" => "http",
                "token" => null));

$uri = $_SERVER["REQUEST_URI"];

$broken = explode("/", $uri);
$call = array_pop($broken);
list($func, $rtype) = explode(".", $call);

// validate the function call is valid
if(!is_callable("api_".$func)) {
	header("HTTP/1.0 404 Not Found");
	header("Content-type: text/plain");	
	echo "the function you requested ($func) was not found on this server.";
	exit;
}

call_user_func("api_".$func);

function api_getcomments() {
	$fileid = check_arg("fileid", "integer", $_REQUEST);
	handle_errors();

	$comments = id(new Upload($_REQUEST["fileid"]))->getComments();

	echo '<div id="comments">';

	$commentnum = 0;
	$numgood = 0;
	$numbad = 0;
	foreach($comments as $comment) {
		if(strlen($comment->text()) == 0) continue;
		$css = $style = $commentnum++ % 2 ? "background:#bbbbee;" : "background:#ccccff";
		require("offensive/templates/comment.inc");
	}
	
	echo '</div>';
}

// dynamic page templating functions
function getlicheck() {
	$fileid = check_arg("fileid", "integer", $_REQUEST);
	handle_errors();
	
	$upload = core_getupload($fileid);
	
	if(array_key_exists("type", $_REQUEST)) {
		if($upload->type() != $_REQUEST["type"]) {
			exit;
		}
	}
	return $upload;
}

function api_getfileli() {
	$upload = getlicheck();
	require("offensive/templates/listitem_file.inc");
	exit;
}

function api_getthumbli() {
	$upload = getlicheck();
	require("offensive/templates/thumbitem_file.inc");
	exit;
}

function api_gettopicli() {
	$upload = getlicheck();
	require("offensive/templates/listitem_topic.inc");
	exit;
}

?>