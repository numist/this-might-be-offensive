<?	
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/functions.inc' );	
	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/core.inc' );
	
	$tmboroot = get_include_path();
	set_include_path($defaultpath);
	require_once("xapian.php");
	set_include_path($tmboroot);

	function start() {
		mustLogIn();
	}
	

	function doSearch( $find ) {
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ? $_REQUEST['p'] : 0;
		// XXX: this should probably use argvalidation
		$numperpage = isset($_REQUEST['limit']) && is_intger($_REQUEST['limit']) ?
			          $_REQUEST['limit'] : 25;
		$start = $page * $numperpage;

		$indexfile = get_include_path()."/offensive/data/comments.db";
		$lockfile = get_include_path()."/offensive/data/comments.db.lock";

		// aquire lock.  try a couple times
		$starttime = microtime(true);
		$fp = fopen($lockfile, "r") or trigger_error("lockfile not found", E_USER_ERROR);
		while(microtime(true) < $starttime + 2 && false === ($locked = flock($fp, LOCK_SH | LOCK_NB))) {
			usleep(round(rand(100, 400) * 1000));
		}
		if(!$locked) {
			trigger_error("locked out.", E_USER_ERROR);
			exit(2);
		}
		
		try {
			$database = new XapianDatabase($indexfile);

		    // Start an enquire session.
		    $enquire = new XapianEnquire($database);

		    // Combine the rest of the command line arguments with spaces between
		    // them, so that simple queries don't have to be quoted at the shell
		    // level.
		    $query_string = $find;

		    $qp = new XapianQueryParser();
		    $stemmer = new XapianStem("english");
		    $qp->set_stemmer($stemmer);
		    $qp->set_database($database);
		    $qp->set_stemming_strategy(XapianQueryParser::STEM_SOME);
		    $query = $qp->parse_query($query_string, XapianQueryParser::FLAG_SPELLING_CORRECTION);
		    echo "<!-- Parsed query is: {$query->get_description()} -->\n";
		
			$corrected = $qp->get_corrected_query_string();
			if($corrected) {
				echo "Did you mean: <a href=\"XXX\">\"$corrected\"</a>?<br />\n";
			}
			
			// get results for the query.
		    $enquire->set_query($query);
		    $matches = $enquire->get_mset($start, $numperpage);

			// end top head with some infos

		    // Display the results.
		    //echo "<br/>".($start + 1)." - ".min($start + $numperpage ,$matches->get_matches_estimated()).
			     " of about {$matches->get_matches_estimated()}.\n\n";
		    
			// XXX: end end end yadda
			?>
			<br />
			<table width="100%">
				<tr>
					<td width="200px">
						<? if( $page > 0 ) { ?>
							<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page - 1 ?>">&laquo;&nbsp;<b>previous&nbsp;page</b></a></td>
						<? } ?>
					</td>
					<td style="text-align:center">
						<?
							// pagination
							echo ($start + 1)." - ".min($start + $numperpage ,$matches->get_matches_estimated()).
							     " of about {$matches->get_matches_estimated()}.\n\n";
						?>
					</td>
					<td style="text-align:right" width="200px">
						<? if( $start + $numperpage < $matches->get_matches_estimated() ) { ?>
							<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page + 1 ?>"><b>next&nbsp;page</b></a>&nbsp;&raquo;
						<? } ?>
					</td>
				</tr>        
			</table>
			
			</div></div><?
		
			// d'oh this is not going to be very efficient, is it…
			
			$results = array();
			$comments = array();
			$i = $matches->begin();
		    while (!$i->equals($matches->end())) {
				$result = array();
				$result['rank'] = $i->get_rank() + 1;
				// XXX: prepopulate comment, upload, username from db
				$result['comment'] = new Comment($i->get_document()->get_data());
				$comments[] = $i->get_document()->get_data();
				$result['match'] = $i->get_percent();
				$i->next();
				$results[] = $result;
			}
			
			// load the class cache
			$comments = core_getcomment($comments);
			
			foreach($comments as $comment) {
				$upload = $comment->upload(); ?>
				<div class="contentbox">
					<div class="heading"><?
					  // changeblog special header
						if($upload->id() == "211604") { ?>
							<a class="heading" id="pic" href="<?= Link::content("changeblog") ?>">changeblog</a>
							<br/><span style="color:#666699">beat to fit. paint to match.</span><br/>
					<? } else if( $upload->type() == 'topic' ) {
							$prefix = $upload->uploader()->status() == "admin" ? "" : "don't blame me,"; ?>
							<a class="heading" id="pic" href="<?= Link::thread($upload) ?>"><?= $upload->htmlFilename(); ?></a>
								<br/><span style="color:#666699"><?= $prefix ?> <a href="<?= Link::user($upload->uploader()) ?>" style="color:#666699"><?= $upload->uploader()->username() ?></a> started it.</span><br/>
					<? } else { ?>
							<a class="heading" id="pic" href="<?= Link::upload($upload) ?>"><?= $upload->htmlFilename(); ?></a><br/><span style="color:#666699">uploaded by <?= $upload->uploader()->htmlUsername() ?></span><br/>
					<? } ?>
					</div>
				<div class="entry">
				<?= $comment->HTMLcomment(); ?><br/>
					 
	 	 			<div class="timestamp"><?= $result['match'] ?>% <a href="<?= Link::comment($comment) ?>"><?= $comment->timestamp() ?></a></div>
					&raquo; <?= $comment->commenter()->htmlUsername() ?>
	
				<?

				if( $comment->vote() ) {
					echo "<span class='vote'> [ " . $comment->vote() . " ]</span>";
				}
							
				if( $comment->tmbo() ) {
					?><span class="vote"> [ this might be offensive ]</span><?php
				}
				
				if( $comment->tiar() ) {
					?><span class="vote"> [ this is a repost ]</span><?php
				}
				?>
				</div></div>
				<?
			}
			
			?>
			
			<div class="contentbox">
				<div class="entry">
			<table width="100%">
				<tr>
					<td width="200px">
						<? if( $page > 0 ) { ?>
							<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page - 1 ?>">&laquo;&nbsp;<b>previous&nbsp;page</b></a></td>
						<? } ?>
					</td>
					<td style="text-align:center">
						<?
							// pagination.
						?>
					</td>
					<td style="text-align:right" width="200px">
						<? if( $start + $numperpage < $matches->get_matches_estimated() ) { ?>
							<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&find=<?= urlencode($_REQUEST['find']) ?>&p=<?= $page + 1 ?>"><b>next&nbsp;page</b></a>&nbsp;&raquo;
						<? } ?>
					</td>
				</tr>        
			</table>

			<?
		
		} catch (Exception $e) {
			flock($fp, LOCK_UN);
			fclose($fp);
		    trigger_error("XAPIAN: ".$e->getMessage(), E_USER_ERROR);
		}
		
		flock($fp, LOCK_UN);
		fclose($fp);
	}

	function body() {
?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">both hands and a flashlight.</div>
	<? tabs(); ?>
	<div class="bluebox">

<?
		$find = trim(sqlEscape(
		        array_key_exists("find", $_REQUEST) ? $_REQUEST['find'] : ""));

		?>
		<div class="entry" style="background:#bbbbee">
				<form action="./">
					<input type="hidden" name="c" value="search"/>
					<input type="text" name="find" value="<?= $find ?>"/>
					<input type="submit" value="search comments"/>				
				</form>
		</div>
		<div class="entry">
		<form action="finduser.php">
			<input type="text" name="finduser" value="<?= array_key_exists("finduser", $_REQUEST) ? $_REQUEST['finduser'] : "" ?>"/>
			<input type="submit" value="find user"/>
		</form>

		</div>
		<div class="entry" style="background:#bbbbee">
		<form action="./">
			<input type="text" name="findfile" value="<?= array_key_exists("findfile", $_REQUEST) ? $_REQUEST['findfile'] : "" ?>"/>
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
	<div class="blackbar"></div>
</div>	
<?
	}
?>