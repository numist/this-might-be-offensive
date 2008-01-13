<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
  
	require_once( 'offensive/assets/tabs.inc' );
	
	function start() {
		mustLogIn();

		setcookie( 'thumbnails', "no", time()-3600, "/offensive/" );
	}

	function body() {
		
		if( file_exists( "./updating.txt" ) ) {
			$updating_now = true;
		}
		
		if( file_exists( "./log.txt" ) ) {
			$last_update = filemtime( "log.txt" );
		} else {
			$last_update = -1;
		}
  
  $page_limit_clause = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] * 100: 0;
    
    // This should be fine, after the key is in memory it is a fast query.
    // We need to be sure that it is using the correct keys if the query is consistently slow
    $sql = "
SELECT
  offensive_uploads.id,
  offensive_uploads.timestamp,
  offensive_uploads.filename,
  offensive_uploads.nsfw,
  offensive_uploads.tmbo,
  offensive_uploads.type,
  offensive_count_cache.good,
  offensive_count_cache.bad,
  offensive_count_cache.tmbo AS tmbo_vote,
  offensive_count_cache.comments,
  users.username
FROM
  offensive_uploads
LEFT JOIN offensive_count_cache
ON  offensive_count_cache.threadid = offensive_uploads.id
JOIN users
ON offensive_uploads.userid = users.userid
WHERE
  offensive_uploads.type= 'image'
AND
  offensive_uploads.status = 'normal'
AND
  users.account_status != 'locked'
ORDER BY offensive_uploads.timestamp DESC, offensive_uploads.id DESC
LIMIT $page_limit_clause, 100;
";

?>
<div class="heading">

	welcome back. we missed you. oh, and don't use apostrophes for plurals. ever.
	<!-- if you don't understand, upload a picture of yer mom and we'll do a little experiment. -->

</div>

<?
	global $activeTab;
	$activeTab = "images";
	tabs();
?>

<div class="bluebox">
		
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td valign="top">
				<? require( 'offensive/assets/pickupLink.inc' ) ?>
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=thumbs">thumbnail view</a></b></div>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
				<table width="100%">
            <?php
            $result = tmbo_query($sql);
            while( $row = mysql_fetch_assoc( $result ) ) 
            {
              $css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
              $filename = $row['filename'];

	      // mark tmbo and nsfw files, if they aren't already
              $filename = $row['tmbo'] == 1 && strpos(strtolower($filename), "[tmbo]") === false ?
	                  '[tmbo] '.$filename : $filename;
              $filename = $row['nsfw'] == 1 && strpos(strtolower($filename), "[nsfw]") === false ?
	                  '[nsfw] '.$filename : $filename;

		// shorten filenames that are too long
              $filename = strlen($filename) > 70 ? substr($filename, 0, 67)."..." : $filename;
	      
	      $filename = htmlEscape($filename);
              ?>
                  <tr class="<?= $css ?>">
                    <td class="<?= $css ?>"><div class="clipper"><a href="pages/pic.php?id=<?= $row['id'] ?>" class="<?= $css ?>" title="uploaded by <?= htmlEscape($row['username']) ?>"><?= $filename ?></a></div></td>
                    <td class="<?= $css ?>" style="text-align:right;white-space:nowrap"><a href="./?c=comments&fileid=<?= $row['id'] ?>" class="<?= $css ?>"><?= $row['comments'] ?> comments</a> (+<?= ($row['good'] != '')?$row['good']:0; ?> -<?= ($row['bad'] != '')?$row['bad']:0;?>)</td>
                  </tr>
                <?php
            }
          ?>
          </table>
			</td>
		</tr>
	</table>
			
</div>

<?
}
?>
