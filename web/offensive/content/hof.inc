<?

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once('offensive/assets/tabs.inc');
	require_once('offensive/assets/functions.inc');
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");
	
	function start() {
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);
	}

	function body() {

		$view = isset( $_REQUEST['t'] ) ? sqlEscape( $_REQUEST['t'] ) : 'hof';	
?>

		<div class="heading">hall of fame: the coolest files of all time (according to you sick puppies).</div>
		<? tabs(); ?>

		<div style="padding-top:8px;background:#ccccff;background-image:url( graphics/subtab_bg.gif );background-position:top left;background-repeat:repeat-x">
			<div class="<?= ($view == 'hof' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof">all time</a></div>
			<div class="<?= ($view == 'today' ? 'tabon' : 'taboff') ?>"><a href="./?c=hof&t=today">past 24 hours</a></div>
			<div class="tabspacer" style="background:none">&nbsp;</div>
		</div>

				
		<div class="bluebox">

			<table width="100%">

			<tr class="<? $css = "evenfile"; echo $css ?>">
				<td style="text-align:right"><b>rank</b></td>
				<td><b>filename</b></td>
				<td><b>uploaded&nbsp;by</b></td>
				<td><b>comments</b></td>
				<td><b>votes</b></td>
				<td><b><? if($view=="today") { ?> hours <? } else { ?> weeks <? } ?></b></td> 
			</tr>

<?
		$numPerPage = 100;
		$page = isset($_REQUEST['p']) && is_numeric($_REQUEST['p']) ?
			    $_REQUEST['p'] : 0;		
		$args = $_REQUEST;
		$args["type"] = "image";
		if(!array_key_exists("sort", $args)) {
			$args["sort"] = "votes_desc";
		}
		if(!array_key_exists("limit", $args)) {
			$args["limit"] = "100";
		}
		
		$result = core_getuploads($args);
		$count = $page * $numPerPage;
		foreach($result as $upload) {
			$css = strpos($css, "even") !== false ? "oddfile odd_row" : "evenfile even_row";
			
			if( $upload->uploader()->id() == $_SESSION['userid'] ) {
				$css = "my_hof ".(strpos($css, "even") !== false ? "even_row" : "odd_row");
			}
			
			if($view=="today") $timeSince=hoursSince($upload->timestamp());
			if($view=="hof") $timeSince = weeksSince($upload->timestamp());
			$cssw = $view == 'hof' ? "$css weeks_$timeSince" : $css;
?>
			<tr class="<? echo $cssw ?>">
				<td style="text-align:right"><? echo ++$count ?>.</td>
				<td><div style="white-space:nowrap;width:312px;overflow:hidden;"><a class="<? echo $cssw ?>" href="pages/pic.php?id=<?= $upload->id() ?>"><?= $upload->htmlFilename()
			    ?></a></td></div>
				<td><a class="<? echo $cssw ?>" href="./?c=user&userid=<?= $upload->uploader()->id() ?>"><?= htmlEscape($upload->uploader()->username()) ?></a></td>
				<td><a class="<? echo $cssw ?>" href="./?c=comments&fileid=<?= $upload->id() ?>"><?= $upload->comments() ?></a></td>
				<td>(+<?= $upload->goods() ?>)</td>
				<td style="text-align:right"><?= $timeSince ?></td>
			</tr>
<?
		}
?>


			</table>


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
