<?php

set_include_path("../..");
// set up the normal TMBO environment
require_once( 'offensive/assets/header.inc' );
require_once( "offensive/assets/activationFunctions.inc" );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();
require_once("offensive/assets/functions.inc");
require_once("offensive/assets/xml.inc");
require_once("offensive/assets/getPrefs.inc");
require_once("offensive/assets/argvalidation.inc");

mustLogIn("http");

$userid = $_SESSION['userid'];
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

// XXX: this function is due for factoring.
function alreadyVoted( $uid, $fid ) {
	$sql = "SELECT count( vote ) AS thecount FROM offensive_comments WHERE fileid=$fid AND userid=$uid AND vote LIKE 'this%'";
	$result = tmbo_query( $sql );
	$row = mysql_fetch_assoc( $result );
                
	$voted = ( $row[ 'thecount' ] > 0 );
	return $voted;
}

// this gets all the HTML for the quickcomment box.
function api_getquickcommentbox() {
	global $userid;

	$fileid = check_arg("fileid", "integer", $method);
	handle_errors();

	// first find out of this is our own posting, which will limit what we can do
	$sql = "SELECT users.username, users.userid, users.account_status, offensive_uploads.filename, nsfw, tmbo, offensive_uploads.timestamp as upload_timestamp, offensive_uploads.type FROM users, offensive_uploads WHERE users.userid = offensive_uploads.userid AND offensive_uploads.id = $fileid";
	$result = tmbo_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$my_posting = ($userid == $row['userid']) ? 1 : 0;

	// check if we've already voted
	$already_voted = alreadyVoted( $userid, $fileid );

	// start building the HTML that will be inserted into the quick comment box directly
?>
		<form id="qc_form">
			<p>
				<input type="hidden" value="<?= $fileid ?>" name="fileid" id="qc_fileid" />
				<textarea cols="64" rows="6" name="comment" id="qc_comment"></textarea>
			</p>
			<div id="qc_vote">
<?php 			if(!$my_posting && !$already_voted) {  // show the vote buttons   ?>
				<ul style="margin-left: -10px; list-style: none;">
					<li><input class="qc_tigtib" style="position: relative; top: 3px;" id="qc_novote" type="radio" value="novote" name="vote"/></li>
                    			<li>[<input class="qc_tigtib" style="position: relative; top: 3px;" id="qc_tig" type="radio" value="this is good" name="vote"/><label for="qc_tig"> this is good </label>]</li>
                  			<li>[<input class="qc_tigtib" style="position: relative; top: 3px;" id="qc_tib" type="radio" value="this is bad" name="vote"/><label for="qc_tib"> this is bad </label>]</li>
         		                <li>[<input style="position: relative; top: 3px;" id="tmbo" type="checkbox" value="omg" name="offensive"/><label for="tmbo"> tmbo </label>]</li>
                  		      	<li>[<input style="position: relative; top: 3px;" id="repost" type="checkbox" value="police" name="repost"/><label for="repost"> tiar </label>]</li>
				</ul>
<?php			} ?>
			</div>
			<div id="qc_go" style="text-align: center">
				<p>
					<input type="submit" value="go" name="submit"/>
				</p>
			</div>
		</form>
		<div id="qc_comments">
<?php	// now fetch all the comments so you can see the comments in the quickcomment box
	$sql = "SELECT offensive_comments.*, offensive_comments.id as commentid, offensive_comments.timestamp AS comment_timestamp, users.* FROM offensive_uploads, offensive_comments, users WHERE users.userid = offensive_comments.userid AND offensive_uploads.id=fileid AND fileid = '$fileid' AND offensive_comments.comment != '' ORDER BY offensive_comments.timestamp ASC";
	$result = tmbo_query( $sql );
	$comments_exist = mysql_num_rows( $result ) > 0;

	if( $comments_exist ) { 
		$comments_heading = "the dorks who came before you said: ";
		if($fileid != "211604") {
			echo "<b>$comments_heading</b>";
		}
		echo '<div id="qc_commentrows">';
		while( $row = mysql_fetch_assoc( $result ) ) {
			extract($row);
			$comment = htmlEscape($comment);
			echo '<div class="qc_comment">';
			if( ! isSquelched( $userid) ) {	
				echo nl2br( linkUrls( $comment ) );
			} else {
				echo "<div class='squelched'>[ squelched ]</div>";
			}
?>
			<br />
			<div class="timestamp"><?= $comment_timestamp ?></div>
			&raquo; <a href="/offensive/?c=user&userid=<?= $userid ?>"><?= $username ?></a>
<?php
			if($vote != '') {
				echo '<span class="vote">[ ' . $vote .' ]</span>';
			}
			if($offensive) {
				echo '<span class="vote"> [ this might be offensive ]</span>';
			}
			if($repost) {
				echo '<span class="vote"> [ this is a repost ]</span>';
			}
			echo '</div>';
		}
		echo "</div>";
	}
}
?>
