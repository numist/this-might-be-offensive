<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
  
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	
	function start() {
		global $me;
		mustLogIn();

		$me = new User($_SESSION["userid"]);
		$me->setPref("index", "");
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

		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$start = $page * $numPerPage;
    
	/* adding the USE KEY statement seems to have resolved performance
	 * issues with this query.  Why MySQL wasn't using the key to
	 * begin with is anyone's guess.  Thanks jonxp for the help.
	 */
    $sql = "SELECT
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
  offensive_uploads USE KEY (t_t_id)
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
LIMIT $start, $numPerPage";

?>
<div class="heading">

	<?
	require("offensive/data/quips.inc");
	echo $quip;
	?>

<?
// XXX: [tpg] — instead of eom, put current tpg winner here.  
// Current [tpg] winner: ________.  See current submissions. (link to findfile [tpg] since (winnerdate))
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
				<? require( 'offensive/assets/pickupLink.inc' ) ?>
			</td>
			<td valign="top">
				<div style="text-align:right"><b><a href="./?c=thumbs">thumbnail view</a></b></div>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
				<table width="100%" id="main">
            <?php
            $result = tmbo_query($sql);
            while( $row = mysql_fetch_assoc( $result ) ) 
            {
              $css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
              $filename = htmlFilename($row);
	      
              ?>
                  <tr class="<?= $css ?>">
                    <td>
							<div class="clipr"><a href="pages/pic.php?id=<?= $row['id'] ?>" class="<?= $css ?>" title="uploaded by <?= htmlEscape($row['username']) ?>"><?= $filename ?></a></div>
					</td>
					<td class="<?= $css ?> com_cnt" style="text-align:right;white-space:nowrap">
						<a href="./?c=comments&fileid=<?= $row['id'] ?>" class="<?= $css ?>"><nobr><?= 
				commentLabel($row['comments'], $row['good'], $row['bad'], $row['tmbo_vote']);
			?></nobr></a>
					</td>
                  </tr>
                <?php
            }
          ?>
          </table>
			</td>
		</tr>
		<tr>
			<td>
				<? if( $page > 0 ) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page - 1 ?>">&laquo; <b>previous page</b></a></td>
				<? } ?>
			</td>
			<td style="text-align:right">
				<? if( mysql_num_rows( $result ) == $numPerPage ) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
				<? } ?>
			</td>
		</tr>
	</table>
			
</div>

<?
}
?>
