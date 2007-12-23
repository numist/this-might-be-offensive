<?

	session_start();
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	function start() {
		if( ! isset( $_SESSION['userid'] ) ){
			header( "Location: ./login.php" );
			exit;
		}

		if( $_REQUEST['submit'] ) {
			$topic = trim( $_REQUEST['topic'] );
			if( strlen( $topic ) > 0 ) {
				$tid = createTopic( $topic, $_SESSION['userid'] );
				
				subscribe( $_SESSION['userid'], $tid );
				
				$comment = trim( $_REQUEST['comment'] );
				if( strlen( $comment ) > 0 && is_numeric( $tid ) ) {
					$commentid = addComment( $tid, $comment, $_SESSION['userid'] );
				}
				
				header( "Location: " . $_SERVER['PHP_SELF'] . "?c=comments&fileid=" . $tid );
			}
		}

	}

	function addComment( $topicid, $comment, $uid ) {
		$link = openDbConnection();
		$sql = "insert into offensive_comments( userid, fileid, comment, user_ip )
				values( $uid, $topicid, '".mysql_real_escape_string($comment)."', '" . $_SERVER['REMOTE_ADDR'] . "' )";
		mysql_query( $sql, $link );
		
		$commentid = mysql_insert_id( $link );
		
		$sql = "insert into offensive_count_cache ( threadid, comments ) VALUES ( $topicid, 1 )
					on duplicate key update comments = comments + 1";
					
		mysql_query( $sql, $link );

		return $commentid;
	}

	function createTopic( $topic, $uid ) {
		
		$link = openDbConnection();
		$sql = "insert into offensive_uploads ( userid, filename, ip, type )
					VALUES ( $uid, '".mysql_real_escape_string($topic)."', '" . $_SERVER['REMOTE_ADDR'] . "', 'topic' )";
					
		mysql_query( $sql, $link );

	 	$threadid = mysql_insert_id( $link );

		return $threadid;

	}

	function subscribe( $uid, $fileid ) {
		$link = openDbConnection();
		
		$sql = "insert into offensive_subscriptions( userid, fileid )
					values( $uid, $fileid )";
					
		$result = mysql_query( $sql );

	}

	require_once( 'tabs.php' );

	function body() {
?>
		<div class="heading">we need to talk.</div>

<?
		global $activeTab;
		
		$activeTab = "discussions";

		tabs();
?>
		<div class="bluebox">
		
			<form method="post" action="<?php echo $_SERVER['PHP_SELF'] ?>">

				<div style="text-align:center">
					<div class="label">about:
					<input type="text" name="topic" size="50" maxlength="50"/>
					<input type="hidden" name="c" value="newtopic"/>					
					</div>
					<textarea name="comment" style="width:80%;height:150px;"></textarea>
					<br/>
					<input type="submit" name="submit" value="go"/>
				</div>
				
			</form>
			
		</div>

<?
	}
?>
