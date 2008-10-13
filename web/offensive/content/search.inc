<?	
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/functions.inc' );	
	require_once( 'offensive/assets/tabs.inc' );

	global $numPerPage;
	$numPerPage = 100;

	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
	}
	
	require_once("Zend/Search/Lucene.php");

	function bsearch($id, $arr, $key) {
		$jump = (int)floor(count($arr) / 2);
		$i = $jump;
		while(true) {
			if($arr[$i][$key] > $id) {
				$i = (int)($i - $jump);
			} else if($arr[$i][$key] < $id) {
				$i = (int)($i + $jump);
			}
			if($arr[$i][$key] == $id) {
				return $arr[$i];
			}
			if($i < 0) $i = 0;
			if($i >= count($arr)) $i = count($arr) - 1;
			
			if($jump <= 1) return false;
			$jump = (int)ceil($jump / 2);
		}
	}

	function doSearch( $find ) {
		global $numPerPage;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;
		$end = ($page + 1) * $numPerPage;
		
		$indexname = get_include_path().'/offensive/data/comments.idx';
		if(!file_exists($indexname)) {
			return;
		} else {
			$index = Zend_Search_Lucene::open($indexname);
		}
		Zend_Search_Lucene::setDefaultSearchField('comment');
		Zend_Search_Lucene_Search_QueryParser::suppressQueryParsingExceptions();
		$query = Zend_Search_Lucene_Search_QueryParser::parse($find);
		$hits  = $index->find($query);
		
		$sql = "SELECT *, offensive_comments.timestamp AS comment_timestamp, offensive_comments.id as commentid
					FROM offensive_comments, offensive_uploads, users
					WHERE offensive_comments.fileid = offensive_uploads.id
					AND offensive_comments.userid = users.userid AND ( false";
		
		for($i = $start; $i < $end && $i < count($hits); $i++) {
			$sql .= " OR offensive_comments.id = ".$hits[$i]->commentid;
		}
		
		$sql .= ") ORDER BY offensive_comments.id ASC";
		
		$result = tmbo_query( $sql );

		?>
					</div>
		<div class="blackbar"></div>
		</div>

		<div class="contentbox">
			<div class="blackbar"></div>
			<div class="heading">results <?= $start + 1 ?> - <?= ($end < count($hits) ? $end : count($hits)) ?> of <?= count($hits) ?>:</div>
			<div class="bluebox">
		<?
		
		/*
		 * RELEVANCE SORTING:
		 * all rows are read into memory at this point, because the database is not aware of 
		 * relevance sorting, and there's no efficient way to teach it.
		 * the solution for the lack of sorting coming out of the database is to read all
		 * results (100 max) from the database, sorted, and do binary searches on the result
		 * set when we need to get a row.  this should still be faster than doing multiple
		 * queries against the database.
		 */
		$rows = array();
		while($row = mysql_fetch_assoc($result)) {
			$rows[] = $row;
		}
		
		// output loop
		for($i = $start; $i < $end && $i < count($hits); $i++) {
			$row = bsearch($hits[$i]->commentid, $rows, "commentid");
			
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
		if($page > 0 || count($hits) > $end) { ?>
		<table width="100%">
			<tr>
				<td>
					<? if( $page > 0 ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
					<? } ?>
				</td>
				<td style="text-align:right">
					<? if( count($hits) > $end ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
					<? } ?>
				</td>
			</tr>        
		</table>
		
		<? }
		
		return;
	}

	function doFile($find) {
		global $numPerPage, $me;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;
		$end = ($page + 1) * $numPerPage;
		
		$indexname = get_include_path().'/offensive/data/uploads.idx';
		if(!file_exists($indexname)) {
			return;
		} else {
			$index = Zend_Search_Lucene::open($indexname);
		}
		Zend_Search_Lucene::setDefaultSearchField('filename');
		Zend_Search_Lucene_Search_QueryParser::suppressQueryParsingExceptions();
		$query = Zend_Search_Lucene_Search_QueryParser::parse($find);
		$hits  = $index->find($query);
		
		$sql = "SELECT offensive_uploads.id,
		               filename,
		               offensive_uploads.timestamp,
					   offensive_uploads.type,
					   offensive_uploads.nsfw,
					   offensive_uploads.tmbo,
					   offensive_uploads.type,
					   users.userid, 
					   username,
					   offensive_count_cache.good, 
					   offensive_count_cache.bad,
					   offensive_count_cache.tmbo as is_tmbo,
					   offensive_count_cache.comments
					FROM offensive_uploads
					JOIN users ON offensive_uploads.userid = users.userid
					JOIN offensive_count_cache ON offensive_count_cache.threadid = offensive_uploads.id
					WHERE false ";
		for($i = $start; $i < $end && $i < count($hits); $i++) {
			$sql .= " OR offensive_uploads.id = ".$hits[$i]->fileid;
		}
		$sql .= " ORDER BY offensive_uploads.id ASC";		
		
		$result = tmbo_query( $sql );

		?>
					</div>
		<div class="blackbar"></div>
		</div>

		<div class="contentbox">
			<div class="blackbar"></div>
			<div class="heading">results <?= $start + 1 ?> - <?= ($end < count($hits) ? $end : count($hits)) ?> of <?= count($hits) ?>:</div>
			<div class="bluebox">
		<?
		
		/*
		 * RELEVANCE SORTING:
		 * all rows are read into memory at this point, because the database is not aware of 
		 * relevance sorting, and there's no efficient way to teach it.
		 * the solution for the lack of sorting coming out of the database is to read all
		 * results (100 max) from the database, sorted, and do binary searches on the result
		 * set when we need to get a row.  this should still be faster than doing multiple
		 * queries against the database.
		 */
		$rows = array();
		while($row = mysql_fetch_assoc($result)) {
			$rows[] = $row;
		}
				
		// output loop
		?><table><?
		for($i = $start; $i < $end && $i < count($hits); $i++) {
			$row = bsearch($hits[$i]->fileid, $rows, "id");
			if($row === false) trigger_error("fuck fuck fuck", E_USER_ERROR);

						$style = (isset($style) && $style == "") ? "background:#bbbbee;" : "";
						$css = (isset($css) && $css == "evenfile") ? "oddfile" : "evenfile";
						$type = $row['type'];

						$filename = htmlFilename($row, $type != 'image');

						$expired = $type == 'image' && getFile($row['id'], $row['filename'], $row['timestamp']) == '' ? "(expired)" : ""
			?>

						<tr class="<? echo $css ?>" style="<?= $style ?>">
							<? if( $type == 'image' || $type == 'avatar') { ?>
								<td width="100px"><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $row['id'] ?>"><?

								if($me->squelched($row["userid"]) || 
								    ($row["nsfw"] == 1 && $me->getPref("hide_nsfw") == 1) || 
								    ($row["tmbo"] == 1 && $me->getPref("hide_tmbo") == 1) ) {
									?>[ filtered ]<?
								} else {
									?><img src="<?= getThumbURL($row['id'], $row['filename'], $row['timestamp'], $row['type']) ?>"/><?
								}
								?></a></td>
								<td><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $row['id'] ?>"><?= $filename ?></a> (uploaded by <?= htmlEscape($row['username']) ?>)<? echo  $expired ?></td>
								<td width="100px" align="center"><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?= 
									($row['comments'] != '')?$row['comments']:0;
								?> comments</a> <br />(+<?=
									($row['good'] != '')?$row['good']:0;
								?> -<?=
									($row['bad'] != '')?$row['bad']:0;
								?> x<?=
									($row['is_tmbo'] != '')?$row['is_tmbo']:0;
								?>)<br /><?=
									date( "Y-m-d", strtotime( $row['timestamp'] ) )
								?></td>
							<? } else if( $type == 'topic' ) { ?>
								<td width="100px"><? echo date( "Y-m-d", strtotime( $row['timestamp'] ) ) ?></td>
								<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?= $filename ?></a></td>
								<td width="100px" align="center"><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?=
									commentLabel($row['comments'], 0, 0, 0, false);
									?></td>
							<? } ?>
						</tr>

			<?

		}
		?></table><?
		if($page > 0 || count($hits) > $end) { ?>
		<table width="100%">
			<tr>
				<td>
					<? if( $page > 0 ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
					<? } ?>
				</td>
				<td style="text-align:right">
					<? if( count($hits) > $end ) { ?>
						<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
					<? } ?>
				</td>
			</tr>        
		</table>
		
		<? }
		
		return;
	}

	function body() {

?>

	<div class="heading">both hands and a flashlight.</div>
	<? tabs(); ?>
	<div class="bluebox">

<?
		$find = (array_key_exists("find", $_REQUEST) ? $_REQUEST['find'] : "");
		$findfile = (array_key_exists("findfile", $_REQUEST) ? $_REQUEST['findfile'] : "");

		?><div class="entry" style="background:#bbbbee">
			<form action="./">
				<input type="hidden" name="c" value="search"/>
				<input type="text" name="find" value="<?= $find ?>"/>
				<input type="submit" value="search comments"/>				
			</form>
		</div>
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
					echo $findfile;
				?>"/>
				<input type="hidden" name="c" value="search"/>
				<input type="submit" value="find file or topic"/>
			</form>
		</div>
		<?
		if( strlen( $find ) > 0 ) {
			doSearch( $find );
		} else if( strlen($findfile) > 0 ) {
			doFile($findfile);
		}
		?></div><?
	}

?>
