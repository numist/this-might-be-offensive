<?	
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	require_once( 'functions.php' );	
	require_once( 'tabs.php' );	

	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			header( "Location: ./?c=mustLogIn" );
		}
	}
	

	function doSearch( $find ) {
		
		$start = 0;
		$end = 50;

		$link = openDbConnection();
		$sql = "SELECT *, offensive_comments.timestamp AS comment_timestamp, offensive_comments.id as commentid
					FROM offensive_comments, offensive_uploads, users
					WHERE MATCH(comment) AGAINST('$find')
					AND offensive_comments.fileid = offensive_uploads.id
					AND offensive_comments.userid = users.userid
					ORDER BY offensive_comments.timestamp DESC
					LIMIT $start, $end
		";

		$result = mysql_query( $sql );
		
		renderResults( $result );
	
	}


	function searchForm( $value ) {
		?>
			<form action="./">
				<input type="hidden" name="c" value="search"/>
				<input type="text" name="find" value="<?= $value ?>"/>
				<input type="submit" value="search comments"/>				
			</form>
		<?
	}

	function renderResults( $result ) {

		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = ($css == "background:#bbbbee;") ? "" : "background:#bbbbee;";
			?>
			<div class="entry" style="<?= $css ?>">
			<?
				$comment = str_replace( array( "&", "<", ">" ), array("&amp;","&lt;","&gt;"), $row['comment'] );

				 echo nl2br( linkUrls( $comment ) ); ?><br/>
				 
 	 			<div class="timestamp"><a href="./?c=comments&fileid=<?= $row['fileid'] ?>#<?= $row['commentid'] ?>"><?= $row['comment_timestamp'] ?></a></div>
				&raquo; 

			<?

			echo "<a href=\"./?c=user&userid=" . $row['userid'] . "\">" . $row['username'] . "</a>";
			if( $row['vote'] ) {
				echo "<span class='vote'> [ " . $row['vote'] . " ]</span>";
			}
						
			if( $row['offensive'] == 1 ) {
				?><span class="vote"> [ this might be offensive ]</span><?php
			}
			
			if( $row['repost'] == 1 ) {
				?><span class="vote"> [ this is a repost ]</span><?php
			}
			?>
			</div>
			<?
		}
	}

	function body() {

?>

	<div class="heading">both hands and a flashlight.</div>
	<? tabs(); ?>
	<div class="bluebox">

<?
		$find = trim( mysql_real_escape_string( $_REQUEST['find'] ) );

		?><div class="entry" style="background:#bbbbee">
		<?
			searchForm( $find );
		?>
		</div>
		<div class="entry">
		<?
			include 'finduserform.php';
		?>
		</div>
		<div class="entry" style="background:#bbbbee">
		<?			
			include 'findfileform.php';
		?>
		</div>
		<?
		if( strlen( $find ) > 0 ) {
			doSearch( $find );
		}
		
		
?>

	</div>

<?
		
	}

?>
