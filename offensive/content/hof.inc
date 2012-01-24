<?

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once('offensive/assets/tabs.inc');
	require_once('offensive/assets/functions.inc');
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
	}

	function head() {
		echo <<<EOT
		<link href="/styles/grid.css?v=1.01" type="text/css" rel="stylesheet">
EOT;
	}
	function body() {

		$view = isset( $_REQUEST['t'] ) ? sqlEscape( $_REQUEST['t'] ) : 'hof';	
?>
<div class="contentbox">
	<div class="blackbar"></div>
		<div class="heading">hall of fame: the coolest files of all time (according to you sick puppies).</div>
		<? tabs(); ?>

		<div style="padding-top:8px;background:#ccccff;background-image:url( graphics/subtab_bg.gif );background-position:top left;background-repeat:repeat-x">
			<div class="<?= ($view == 'hof' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof">all time</a></div>
			<div class="<?= ($view == 'today' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof&t=today">past 24 hours</a></div>
			<div class="tabspacer" style="background:none">&nbsp;</div>
		</div>


		<div class="bluebox">
			<div id="grid-container">
				<div class="grid" id="hof">
					<ul>
						<li class="header">
							<div class="col col1">rank</div>
							<div class="col col2">filename</div>
							<div class="col col3">uploaded by</div>
							<div class="col col4">comments</div>
							<div class="col col5">votes</div>
							<div class="col col6"><? if($view=="today") { ?> hours <? } else { ?> weeks <? } ?></div>
						</li>

<?
		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_intger($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;
		$args = $_REQUEST;

		if(!array_key_exists("sort", $args)) {
			$args["sort"] = "votes_desc";
		}
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = "100";
		}
		if($view == "today") {
			$args["after"] = time() - (60 * 60 * 24);
		}
		if(!array_key_exists("type", $args)) {
			$args["type"] = "image";
		}

		$result = core_getuploads($args);
		$count = $page * $numPerPage;
		$css = "oddfile odd_row";
		foreach($result as $upload) {
			// cellophane doesn't want her images in the hof;
			// this is a quick hack to omit them.
			if( $upload->uploader()->id() == 1138 ) {
				continue;
			}
			$css = strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";

			if( $upload->uploader()->id() == me()->id()) {
				$css = "my_hof ".(strpos($css, "even") !== false ? "even_row" : "odd_row");
			}

			if($view=="today") $timeSince=hoursSince($upload->timestamp());
			if($view=="hof") $timeSince = weeksSince($upload->timestamp());
			$cssw = $view == 'hof' ? "$css weeks_$timeSince" : $css;
?>
						<li fileid="<?= $upload->id() ?>">
							<div class="col col1 <?= $cssw ?>">
								<? echo ++$count ?>.
							</div>
							<div class="col col2 <?= $cssw ?>">
								<a href="pages/pic.php?id=<?= $upload->id() ?>"><?= $upload->htmlFilename()?></a>
							</div>
							<div class="col col3 <?= $cssw ?>">
								<?= $upload->uploader()->htmlUsername(); ?>
							</div>
							<div class="col col4 <?= $cssw ?>">
								<a href="./?c=comments&fileid=<?= $upload->id() ?>"><?= $upload->comments() ?></a>
							</div>
							<div class="col col5 <?= $cssw ?>">
								+<?= $upload->goods() ?>
							</div>
							<div class="col col6 <?= $cssw ?>">
								<?= $timeSince ?>
							</div>
						</li>
						<?
								}
						?>
					</ul>
				</div>
				<div id="bottom">
					<p><a href="">load more…</a></p>
				</div>
			</div>
	</div>
	<div class="blackbar"></div>
</div>
<?
	}

	function weeksSince( $timestamp ) {
		return floor( (time() - strtotime( $timestamp )) / (60 * 60 * 24 * 7) );
	}

	function hoursSince( $timestamp) {
		return floor((time() - strtotime($timestamp)) / (60*60));
	}

?>
