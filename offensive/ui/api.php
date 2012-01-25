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

// this gets all the HTML for the quickcomment box.
function api_getquickcommentbox() {
	global $userid;

	$fileid = check_arg("fileid", "integer", $_REQUEST);
	handle_errors();

	// first find out of this is our own posting, which will limit what we can do
	$sql = "SELECT users.username, users.userid, users.account_status, offensive_uploads.filename, nsfw, tmbo, offensive_uploads.timestamp as upload_timestamp, offensive_uploads.type FROM users, offensive_uploads WHERE users.userid = offensive_uploads.userid AND offensive_uploads.id = $fileid";
	$result = tmbo_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$my_posting = ($userid == $row['userid']) ? 1 : 0;
	$upload = new Upload($fileid);

	// start building the HTML that will be inserted into the quick comment box directly
	?>
	<a name="form"></a>
	<form id="qc_form">
		<p>
			<input type="hidden" value="<?= $fileid ?>" name="fileid" id="qc_fileid" />
			<input type="hidden" name="c" value="comments"/>
			<textarea cols="64" rows="6" name="comment" id="qc_comment"></textarea>
		</p>
					
		<? if( canVote($upload->id()) && $upload->file() && $upload->type() != 'topic' ) { ?>
			<div id="qc_vote" style="text-align:left;margin-left:14%">
				<table><tr><td width="200px">
				<input class="qc_tigtib" id="qc_novote" type="radio" value="novote" name="vote" checked />
				<br />
				
				<input class="qc_tigtib" type="radio" name="vote" value="this is good" id="qc_tig"/>
				<label for="qc_tig">[ this is good ]</label><br/>
					
				<input class="qc_tigtib" type="radio" name="vote" value="this is bad" id="qc_tib"/>
				<label for="qc_tib">[ this is bad ]</label><br/>
				</td>
				<td>
				<input type="checkbox" name="offensive" value="omg" id="tmbo"/>
				<label for="tmbo">[ this might be offensive ]</label><br/>
			
				<input type="checkbox" name="repost" value="police" id="repost"/>
				<label for="repost">[ this is a repost ]</label><br/>
				<input type="checkbox" name="subscribe" value="subscribe" id="subscribe"/>
				<label for="subscribe">[ subscribe ]</label><br/>
				</td></tr></table>
	
			</div>
		<? } ?>
		<div id="qc_go" style="text-align: center">
			<p>
				<input type="submit" name="submit" value="go"/>
			</p>
		</div>
	</form>
	<div id="qc_comments">
	<?php	// now fetch all the comments so you can see the comments in the quickcomment box
	$comments = $upload->getComments();
	$comments_exist = false;
	foreach($comments as $comment) {
		if($comment->text() != '') {
			$comments_exist = true;
			break;
		}
	}
	

	if( $comments_exist ) {
		$i = 0;
		$comments_heading = "the dorks who came before you said: ";
		if($fileid != "211604") {
			echo "<b>$comments_heading</b>";
		}
		echo '<div id="qc_commentrows">';
		foreach($comments as $comment) {
			if($comment->text() == '')
				continue;
			$commenter = $comment->commenter();
			echo '<div class="entry u'.$commenter->id().'" style="'.($i++ % 2 ? "background:#bbbbee;" : "background:#ccccff").'">';
			echo $comment->HTMLcomment();
?>
			<br />
			<div class="timestamp"><?= $comment->timestamp() ?></div>
			&raquo; <?= $commenter->htmlUsername() ?>
<?php
			if($comment->vote() != '') {
				echo '<span class="vote"> [ ' . $comment->vote() .' ]</span>';
			}
			if($comment->tmbo()) {
				echo '<span class="vote"> [ this might be offensive ]</span>';
			}
			if($comment->tiar()) {
				echo '<span class="vote"> [ this is a repost ]</span>';
			}
			echo '</div>';
		}
		echo "</div>";
	}
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