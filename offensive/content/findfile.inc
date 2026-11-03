<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once('offensive/assets/classes.inc');

	function start() {
		mustLogIn();
		
		if(!isset($_REQUEST['findfile']))
			header("Location: ./");
	}

	function body() {
// XXX: _REQUEST['since'] gives a lower timestamp bound -- I forget why I wanted this...

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;
		$find = $_REQUEST['findfile'];
	
		$sql = "SELECT offensive_uploads.*,
					users.username AS user_username,
					offensive_count_cache.good AS goods, offensive_count_cache.bad AS bads,
					offensive_count_cache.tmbo AS tmbos, offensive_count_cache.comments
					FROM offensive_uploads, users, offensive_count_cache
					WHERE offensive_uploads.filename LIKE '%".sqlEscape($find)."%'
					AND offensive_uploads.userid = users.userid
					AND offensive_count_cache.threadid = offensive_uploads.id
					AND offensive_uploads.status != 'pending'
					ORDER BY offensive_uploads.timestamp DESC
					LIMIT $start, $numPerPage;
				";

		$result = tmbo_query($sql, 5);
		
		
?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">search results</div>
	<div class="bluebox">
<?php
		if( mysqli_num_rows( $result ) == 0 ) {
			?><p>No dice, but thanks for playing.</p><?php
		} else {
			?><ul class="imagelist"><?php
		}

		while( $row = mysqli_fetch_assoc( $result ) ) {
			$upload = new Upload($row);

			$filename = $upload->htmlFilename($upload->type() != 'image');
			$date = date( "Y-m-d", strtotime( $upload->timestamp() ) );
?>
			<li>
				<?php if( $upload->type() == 'topic') { ?>
					<div><a href="<?= Link::thread($upload) ?>"><?= $filename ?></a></div>
					<div>
						<a href="<?= Link::thread($upload) ?>"><?= $upload->commentLabel(false) ?></a><br />
						<?= $date ?>
					</div>
				<?php } else { ?>
					<div>
						<a href="<?= Link::upload($upload) ?>"
							<?php if($upload->filtered()) { ?>
								onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
								onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
							<?php } ?>
						><img name="th<?= $upload->id()?>" alt=""
							src="<?= $upload->filtered()
								? "/offensive/graphics/th-filtered.gif"
								: $upload->thumbURL() ?>"
						/></a><br />
						<a href="<?= Link::upload($upload) ?>"><?= $filename ?></a>
						(uploaded by <?= htmlEscape($upload->uploader()->username()) ?>)
						<?= $upload->expired() ? "(expired)" : "" ?>
					</div>
					<div>
						<a href="<?= Link::thread($upload) ?>"><?= $upload->commentLabel() ?></a><br />
						<?= $date ?>
					</div>
				<?php } ?>
			</li>
<?php
		}

		if( mysqli_num_rows( $result ) > 0 ) {
			?></ul><?php
		}

		$pageLink = function($p) use ($find) {
			return Link::content("findfile")."&".http_build_query(array("findfile" => $find, "p" => $p));
		};
?>
		<div id="commands">
			<div>
				<?php if( $page > 0 ) { ?>
					<a href="<?= htmlEscape($pageLink($page - 1)) ?>">&laquo; <b>previous page</b></a>
				<?php } ?>
			</div>
			<div>
				<?php if( mysqli_num_rows( $result ) == $numPerPage ) { ?>
					<a href="<?= htmlEscape($pageLink($page + 1)) ?>"><b>next page</b></a> &raquo;
				<?php } ?>
			</div>
		</div>
	</div>
	<div class="blackbar"></div>
</div>
<?php

	}

?>