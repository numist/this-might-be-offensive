<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
  
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	
	function start() {
		global $me;
		mustLogIn();

		$me = new User($_SESSION["userid"]);
		$me->setPref("index", "");
	}
	
	function head() {
		echo <<<EOT
		<link id="gallery" rel="alternate" href="/offensive/audio_rss.php" type="application/rss+xml" title="audio stream" />
		<link rel="alternate" type="application/rss+xml" title="daily archives" href="/offensive/zip_rss.php" />
EOT;
	}

	function body() {
		$numPerPage = 100;

		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;		
		$args = $_REQUEST;
		
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = $numPerPage;
		}
		if(!array_key_exists("type", $args)) {
			$args["type"] = "audio";
		}

?>
<div class="heading">

	oh lookie here...

</div>

<?
	global $activeTab;
	$activeTab = "audio";
	tabs();
?>

<div class="bluebox">
		
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td valign="top" colspan="2">
				<? require( 'offensive/assets/pickupLink.inc' ) ?>
			</td>
		</tr>
		<tr>
			<td valign="top" colspan="2">
				<table width="100%" id="main">
            <?php
        	$result = core_getuploads($args);
        	foreach( $result as $upload ) {
              	$css = (isset($css) && $css == "odd_row") ? "even_row" : "odd_row";
              	$filename = $upload->htmlFilename();
	      
              	?>
                  	<tr class="<?= $css ?>">
                    <td>
						<div class="clipr"><a href="pages/pic.php?id=<?= $upload->id() ?>" class="<?= $css ?>" title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>"><?= $upload->htmlFilename() ?></a>
							<?
							if(!file_exists($upload->file())) {
								echo "(expired)";
							}
							?>
						</div>
					</td>
					<td class="<?= $css ?> com_cnt" style="text-align:right;white-space:nowrap">
						<a href="./?c=comments&fileid=<?= $upload->id() ?>" class="<?= $css ?>"><nobr><?= 
				$upload->commentLabel();
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
				<? if(count($result) == $numPerPage) { ?>
					<a href="<?= $_SERVER['PHP_SELF'] ?>?c=<?= $_REQUEST['c'] ?>&p=<?= $page + 1 ?>"><b>next page</b></a> &raquo;
				<? } ?>
			</td>
		</tr>
	</table>
			
</div>

<?
}
?>