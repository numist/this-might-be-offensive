<?	
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/functions.inc' );	
	require_once( 'offensive/assets/tabs.inc' );

	function start() {
		global $me;
		mustLogIn();

		$me = new User($_SESSION["userid"]);
	}
	

	function doSearch( $find ) {
		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;

		$sql = "SELECT *, offensive_comments.timestamp AS comment_timestamp, offensive_comments.id as commentid
					FROM offensive_comments, offensive_uploads, users
					WHERE MATCH(comment) AGAINST('".sqlEscape($find)."' IN BOOLEAN MODE)
					AND offensive_comments.fileid = offensive_uploads.id
					AND offensive_comments.userid = users.userid
					ORDER BY offensive_comments.timestamp DESC
					LIMIT $start, $numPerPage
		";

		$result = tmbo_query( $sql );
		
		renderResults( $result, $numPerPage, $page );
	
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

	function renderResults( $result, $numPerPage, $page ) {
		while( $row = mysql_fetch_assoc( $result ) ) {
			$css = (isset($css) && $css == "") ? "background:#bbbbee;" : "";
			?>
			<div class="entry" style="<?= $css ?>">
			<?
				$comment = htmlEscape($row['comment']);

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
		?>
		<table width="100%">
			<tr>
				<td>
					<? if( $page > 0 ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
					<? } ?>
				</td>
				<td style="text-align:right">
					<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
					<? } ?>
				</td>
			</tr>        
		</table>
		
		<?
	}

	function body() {
	box("<a href='/offensive/index.php?c=comments&fileid=245324'>comment search is dead</a> right now (long live comment search)");
?>

	<div class="heading">both hands and a flashlight.</div>
	<? tabs(); ?>
	<div class="bluebox">

<?
		$find = trim(sqlEscape(
		        array_key_exists("find", $_REQUEST) ? $_REQUEST['find'] : ""));

		?><!--<div class="entry" style="background:#bbbbee">
		<?
			searchForm( $find );
		?>
		</div>-->
		<div class="entry">
		<form action="finduser.php">
			<input type="text" name="finduser" value="<?
				echo (array_key_exists("finduser", $_REQUEST) ? 
				$_REQUEST['finduser'] : ""); 
			?>"/>
			<input type="submit" value="find user"/>
		</form>

		</div>
		<div class="entry" style="background:#bbbbee">
		<form action="./">
			<input type="text" name="findfile" value="<?
				echo (array_key_exists("findfile", $_REQUEST) ?
				$_REQUEST['findfile'] : "");
			?>"/>
			<input type="hidden" name="c" value="findfile"/>
			<input type="submit" value="find file or topic"/>
		</form>
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
