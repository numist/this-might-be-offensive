<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
  
	require_once( 'offensive/assets/tabs.inc' );

	function start() {
		setcookie( 'thumbnails', "yes", time()+3600*24*365*10, "/offensive/" );
	}

	function body() {
  
		$THUMBS_PER_ROW = 4;
    
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

        <!-- welcome back. we missed you. oh, and don't use apostrophes for plurals. ever. -->
        if you don't understand, upload a picture of yer mom and we'll do a little experiment.

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
				<? include( 'offensive/assets/pickupLink.inc' ) ?>
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=main">text view</a></b></div>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
			          <table width="100%" class="thumbnails">
            <?php
            $result = tmbo_query($sql);
            $output = 0;
            while( $row = mysql_fetch_assoc( $result ) ) 
            {
              $css = $row['nsfw'] == 1 ? ' class="nsfw"' : "";
              $filename = rawurlencode($row['filename']);
              	if( $output % $THUMBS_PER_ROW == 0 ) {
                  ?><tr><?php
                }
              ?>
                    <td>

	<a href="pages/pic.php?id=<?= $row['id'] ?>"><span<?= $css?>><img src="<?
		if(file_exists("images/thumbs/th-".$row['filename'])) {
			echo "images/thumbs/th-$filename";
		} else {
			echo "graphics/previewNotAvailable.gif";
		} 
		?>" border="0" alt="uploaded by <?= htmlEscape($row['username']) ?>"/></span></a><br/>
	<a href="./?c=comments&fileid=<?= $row['id'] ?>"><?= $row['comments'] ?> comments (+<?= ($row['good'] != '')?$row['good']:0; ?> -<?= ($row['bad'] != '')?$row['bad']:0;?>)</a>
</td>
                <?php
                if( $output % $THUMBS_PER_ROW == $THUMBS_PER_ROW - 1 ) {
                ?></tr><?php
                }
                $output++;
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
