<?
	// Include, and check we've got a connection to the database.
	include_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();  
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");

	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);

		if(!array_key_exists("userid", $_REQUEST))
			$me->setPref("index", "thumbs");
	}
	
	function head() {
		echo <<<EOT
		<link id="gallery" rel="alternate" href="/offensive/pic_rss.php?gallery=true" type="application/rss+xml" title="PicLens feed" />
		<link rel="alternate" type="application/rss+xml" title="image stream" href="/offensive/pic_rss.php" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="/offensive/zip_rss.php" />
		<link rel="alternate" type="application/rss+xml" title="discussions" href="/offensive/discuss_rss.php" />
EOT;
	}

	function body() {
  
		$THUMBS_PER_ROW = 4;

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;

		$userid = isset($_REQUEST['userid']) && is_numeric($_REQUEST['userid']) ? $_REQUEST['userid'] : 0;
		$userid_sql = $userid ? " AND users.userid = '$userid' " : " AND users.account_status != 'locked' ";
    
	/* adding the USE KEY statement seems to have resolved performance
	 * issues with this query.  Why MySQL wasn't using the key to
	 * begin with is anyone's guess.  Thanks jonxp for the help.
	 */
    $sql = "SELECT
  offensive_uploads.*,
  offensive_count_cache.good AS goods,
  offensive_count_cache.bad AS bads,
  offensive_count_cache.tmbo AS tmbos,
  offensive_count_cache.comments,
  users.username AS user_username,
  offensive_uploads.userid AS user_userid
FROM
  offensive_uploads USE KEY (t_t_id)
LEFT JOIN offensive_count_cache
ON  offensive_count_cache.threadid = offensive_uploads.id
JOIN users
ON offensive_uploads.userid = users.userid
WHERE
  offensive_uploads.type= 'image'
AND
  offensive_uploads.status = 'normal'
$userid_sql
ORDER BY offensive_uploads.timestamp DESC, offensive_uploads.id DESC
LIMIT $start, $numPerPage
";
?>
<div class="heading">

        <?
		require("offensive/data/quips.inc");
		echo $quip;
		?>

<?
	// employee of the month insertion
    $employee = get_include_path()."/offensive/employeeOfTheMonth.txt";
    if(file_exists($employee) && time() - filemtime($employee) < 172800) {
        require("offensive/employeeOfTheMonth.txt");
    }
?>

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
				$upload = new Upload($row);
				
             	$css = $upload->filtered() ? ' class="filtered"' : "";
              	$filename = rawurlencode($row['filename']);
              	if( $output % $THUMBS_PER_ROW == 0 ) {
                  ?><tr><?php
                }
              ?>
                    <td>
	
	<a href="pages/pic.php?id=<?= $upload->id() ?>"
		<? if($upload->filtered()) { ?>
			onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
	 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
		<? } ?>
	><img name="th<?= $upload->id()?>"
		src="<?= $upload->filtered()
			? "/offensive/graphics/th-filtered.gif" 
			: $upload->thumbURL() ?>"
		title="uploaded by <?= $upload->uploader()->username(); ?>"
	/></a>
	
	
	
	<br/>
	<a href="./?c=comments&fileid=<?= $upload->id() ?>"><?= 
		$upload->commentLabel();
	?></a>
</td>
                <?php
                if( $output % $THUMBS_PER_ROW == $THUMBS_PER_ROW - 1 ) {
                ?></tr><?php
                }
                $output++;
            }
            if( $output % $THUMBS_PER_ROW ) {
            ?></tr><?php
            }
            $output++;
          ?>
          </table>
</td>
		</tr>
		<tr>
			<td>
				<? 
				$args = isset($_REQUEST['userid']) ? "&userid=".$_REQUEST['userid'] :'';
				if( $page > 0 ) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'].$args ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
				<? } ?>
			</td>
			<td style="text-align:right">
				<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'].$args ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
				<? } ?>
			</td>
		</tr>
	</table>

</div>

<?
}
?>