<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );

	function start() {
		mustLogIn();

		if( array_key_exists("submit", $_REQUEST) ) {
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
		$sql = "insert into offensive_comments( userid, fileid, comment, user_ip )
				values( $uid, $topicid, '".sqlEscape($comment)."', '" . $_SERVER['REMOTE_ADDR'] . "' )";
		tmbo_query($sql);
		
		$commentid = mysql_insert_id();
		
		$sql = "insert into offensive_count_cache ( threadid, comments ) VALUES ( $topicid, 1 )
					on duplicate key update comments = comments + 1";
					
		tmbo_query($sql);

		return $commentid;
	}

	function createTopic( $topic, $uid ) {
		$sql = "insert into offensive_uploads ( userid, filename, ip, type )
					VALUES ( $uid, '".sqlEscape($topic)."', '" . $_SERVER['REMOTE_ADDR'] . "', 'topic' )";
					
		tmbo_query($sql);

	 	$threadid = mysql_insert_id();

		return $threadid;

	}

	function subscribe( $uid, $fileid ) {
		$sql = "SELECT * FROM offensive_subscriptions WHERE userid = $uid AND fileid = $fileid";
		if(mysql_num_rows(tmbo_query($sql)) > 0) return;

		$sql = "insert into offensive_subscriptions( userid, fileid )
					values( $uid, $fileid )";
					
		$result = tmbo_query( $sql );

	}

	require_once( 'offensive/assets/tabs.inc' );

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