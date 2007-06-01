<?php
	session_start();
	
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'tabs.php' );	

	function addBookmarks( $fileid ) {
		$link = openDbConnection();
		
		# this ends up creating multiple bookmarks for the same user.
		$sql = "INSERT INTO offensive_bookmarks (userid, fileid )
					SELECT DISTINCT userid, fileid
					FROM offensive_comments
					WHERE fileid = $fileid
					AND comment <> ''";
					
		$result = mysql_query( $sql );
		
		$sql = "INSERT INTO offensive_bookmarks ( userid, fileid ) 
					SELECT userid, id
					from offensive_uploads
					where id = $fileid";
					
		$result = mysql_query( $sql );

	}

	function alreadyVoted( $uid, $fid ) {
		$sql = "SELECT count( vote ) AS thecount FROM offensive_comments WHERE fileid=$fid AND userid=$uid AND vote LIKE 'this%'";
		$result = mysql_query( $sql );
		$row = mysql_fetch_assoc( $result );
		
		$voted = ( $row[ 'thecount' ] > 0 );
		return $voted;
	}
	
	function clearBookmarks( $userid, $fileid ) {
		if( (! is_numeric( $userid ) ) || (! is_numeric( $fileid ) ) ) {
			return;
		}
		$link = openDbConnection();
		$sql = "DELETE FROM offensive_bookmarks WHERE userid=$userid and fileid=$fileid";
		$result = mysql_query( $sql );
	}

	function start() {
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./login.php" );
			exit;
		}
	
	
		if( ! is_numeric( $_REQUEST['fileid'] ) ){
			header( "Location: ./" );
			exit;
		}
		
		$usrid = $_SESSION['userid'];
		$fileid = $_REQUEST['fileid'];
		
		clearBookmarks( $usrid, $fileid );

		$link = openDbConnection();
		
	
		if( $_REQUEST['submit'] ) {

			if( ! is_numeric( $_REQUEST['fileid'] ) ) {
				header( "Location: ./" );
			}

			$comment = trim( $_REQUEST['comment'] );
			
			$already_voted = alreadyVoted( $_SESSION['userid'], $_REQUEST['fileid'] );
						
			$sql = "SELECT userid FROM offensive_uploads WHERE id=" . $_REQUEST['fileid'];
			$row = mysql_fetch_assoc( mysql_query( $sql ) );
			if( $already_voted || $row['userid'] == $_SESSION['userid'] || $expired ) {
				$vote = null;
			}
			else {
				$vote = $_REQUEST['vote'];
			}

			$offensive = $_REQUEST['offensive'] != "" ? 1 : 0;
			$repost = $_REQUEST['repost'] != "" ? 1 : 0;			
			$sql = "INSERT INTO offensive_comments ( userid, fileid, comment, vote, offensive, repost, user_ip )";
			$sql .= " VALUES ( $usrid, $fileid, '$comment', '$vote', $offensive, $repost, '" . $_SERVER['REMOTE_ADDR'] . "')";
		
			if( ! ($comment == "" && $vote == null && $offensive == 0 && $repost == 0) ) {
				$result = mysql_query( $sql );
			}

			if( strlen( $comment ) > 0 ) {
				addBookmarks( $fileid );
			}

			
			if( $_REQUEST['redirect'] ) {
				header( "Location: /offensive/pages/pic.php?id=" . $_REQUEST['fileid'] );
				exit;
			}
			
			// redirecting to the same place prevents 'reload' from reposting the comment.
			header( "Location: " . $_SERVER['PHP_SELF'] . "?c=comments&fileid=" . $_REQUEST['fileid'] );
			exit;
	
		
		}
		
	}

	function head() {
?>
	<script type="text/javascript">
		
		function handleKeyDown( e ) {

			if( e == null || e.which == null ) {
				return true;
			}
			
			var id;
			
			switch( e.which ) {
			
				case 38:
					id = "pic";
				break;

			}
			
			if( id ) {
				document.location.href = document.getElementById( id ).href;
			}
			return false;
		
		}
		
	</script>
	
<?
	}

	function body() {
	
	$link = openDbConnection();

	// get the uploader's name
	$sql = "SELECT (DATE_ADD( offensive_uploads.timestamp, INTERVAL 4 DAY ) < now() ) AS expired,
			users.username, users.userid, offensive_uploads.filename, offensive_uploads.timestamp as upload_timestamp, offensive_uploads.type
				FROM users, offensive_uploads
				WHERE users.userid = offensive_uploads.userid
				AND offensive_uploads.id = " . $_REQUEST['fileid'];
	$result = mysql_query( $sql );
	$row = mysql_fetch_assoc( $result );
	$uploader = $row['username'];
	$filename = $row['filename'];
	$uploaderid = $row['userid'];
	$type = $row['type'];
	$expired = $row['expired'] == 1 ? true : false;

	$already_voted = $_SESSION['userid'] == $row['userid'];
	
	$sql = "SELECT offensive_uploads.filename AS filename, offensive_comments.*, offensive_comments.id as commentid, offensive_comments.timestamp AS comment_timestamp, users.*
				FROM offensive_uploads, offensive_comments, users
				WHERE users.userid = offensive_comments.userid
				AND offensive_uploads.id=fileid AND fileid = " . $_REQUEST['fileid'] . " 
				ORDER BY offensive_comments.timestamp";
	
	$result = mysql_query( $sql );
	
	$comments_exist = mysql_num_rows( $result ) > 0;
	
	$comments_heading = "the dorks who came before you said:";
	$add_comment_heading = $comments_exist ? "and then you came along and were all:" : "you were first on the scene and were all:" 

?>	


		<div class="heading">
<?
	if( $type == 'topic' ) {
?>
		<?php echo $filename?><br/><span style="color:#666699">don't blame me. <a href="./?c=user&userid=<?echo $uploaderid ?>" style="color:#666699"><?php echo $uploader?></a> started it.</span><br/>
<?
	}
	else {
?>
		<a class="heading" id="pic" href="pages/pic.php?id=<?php echo $_REQUEST['fileid']?>"><?php echo $filename?></a><br/><span style="color:#666699">uploaded by <a href="./?c=user&userid=<?echo $uploaderid ?>" style="color:#666699"><?php echo $uploader?></a></span><br/>
<?
	}	
?>
			
		</div>
	
<?
		global $activeTab;
		$activeTab = "discussions";
		tabs();
?>
	
		<div class="bluebox" style="text-align:left">	

<?php if( $comments_exist ) { ?>
		
			
		<b><?php echo $comments_heading?></b>
<?php


	
	while( $row = mysql_fetch_assoc( $result ) ) {
	?>
	<a name="<?= $row['commentid'] ?>"></a>
	<div class="entry" style="<?php echo nextStyle()?>">

			<?php
			
				$comment = str_replace( array( "&", "<", ">" ), array("&amp;","&lt;","&gt;"), $row['comment'] );
	
	
				 echo nl2br( $comment ); ?><br/>
 	 			<div class="timestamp"><a href="#<?= $row['commentid'] ?>"><?= $row['comment_timestamp'] ?></a></div>
				&raquo; 

			<?php

			echo "<a href=\"./?c=user&userid=" . $row['userid'] . "\">" . $row['username'] . "</a>";
			if( $row['vote'] ) {
				echo "<span class='vote'> [ " . $row['vote'] . " ]</span>";
				if( $row['userid'] == $_SESSION['userid'] ) {
					$already_voted = true;
				}		
			}
			
			
			if( $row['offensive'] == 1 ) {
				?><span class="vote"> [ this might be offensive ]</span><?php
			}
			
			if( $row['repost'] == 1 ) {
				?><span class="vote"> [ this is a repost ]</span><?php
			}

			?>
	</div>	
	
	<?php } ?>

	
	<?php } ?>

			</div>
	</div>

	

	<div class="contentbox">
		<div class="blackbar"></div>
			<div class="heading"><?php echo $add_comment_heading?></div>
			<div class="bluebox" style="text-align:center">			
			
			<form method="post" action="<?php echo $_SERVER['PHP_SELF']?>">

			
				<p>
					<input type="hidden" name="fileid" value="<?php echo $_REQUEST['fileid']?>"/>
					<input type="hidden" name="c" value="comments"/>
					<textarea name="comment" style="width:80%;height:150px;"></textarea>
				</p>
				


<?php 

	if( ! $already_voted && ! $expired && $type != 'topic' ) {
	
	?>	<div style="text-align:left;margin-left:10%"><?php

		// show vote options	
		
		$sql = "SHOW COLUMNS FROM offensive_comments LIKE 'vote'";
		$result = mysql_query( $sql );
		$row = mysql_fetch_row($result);
		$options = explode("','",preg_replace("/(enum|set)\('(.+?)'\)/","\\2",$row[1]));
		
		
		foreach( $options as $option ) {
			?>
				<input type="radio" name="vote" value="<?php echo $option ?>" id="<?php echo $option?>"/>
				<label for="<?php echo $option?>">[ <?php echo $option?> ]</label><br/>
			
			<?php
		}
	?>

		<br/>		

		<input type="checkbox" name="offensive" value="omg" id="tmbo"/>
		<label for="tmbo">[ this might be offensive ]</label><br/>
		
		<input type="checkbox" name="repost" value="police" id="repost"/>
		<label for="repost">[ this is a repost ]</label><br/>

</div>

	<?php

	}
?>

						
				<p>
					<input type="submit" name="submit" value="go"/>
				</p>
				
				</form>
			</div>

<?
}

	function nextStyle() {
		global $style;
		$style = $style == "background:#bbbbee;" ? "" : "background:#bbbbee;";
		return $style;
	}


?>