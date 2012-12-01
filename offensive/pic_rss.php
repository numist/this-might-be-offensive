<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

mustLogIn(array("prompt" => "http",
                "token" => null));

require_once("offensive/assets/conditionalGet.inc");
require_once("offensive/assets/functions.inc");

require_once("offensive/assets/classes.inc");
require_once("offensive/assets/core.inc");

$sql = "SELECT offensive_uploads.timestamp
		FROM offensive_uploads USE KEY (t_t_id)
			LEFT JOIN users ON offensive_uploads.userid = users.userid
		WHERE type='image' AND status='normal'
 		ORDER BY timestamp DESC
		LIMIT 1";
$res = tmbo_query($sql);
$row = mysql_fetch_array($res);
$lastBuildDate = array_pop($row);
$lastBuildTime = strtotime($lastBuildDate);
conditionalGet($lastBuildTime);

?>
<rss <? if( isset($_GET['gallery']) ) { echo 'xmlns:media="http://search.yahoo.com/mrss"'; } ?> version="2.0">
	<channel>
		<? if( ! isset($_GET['gallery']) ) { ?>
			<title>[ this might be offensive ] : images</title>
			<link>https://<?= $_SERVER['HTTP_HOST'] ?>/offensive/</link>
			<description>[ this might be offensive ]</description>
			<lastBuildDate><?= gmdate('r', $lastBuildTime);	?></lastBuildDate>
		<? } 
	
    $args = $_REQUEST;
	if(!array_key_exists("type", $args)) {
		$args["type"] = "image";
	}

	$result = core_getuploads($args);

	foreach( $result as $upload ) {
		// mark tmbo and nsfw files, if they aren't already
		$filename = $upload->filename();
		$filename = $upload->is_tmbo() == 1 && strpos(strtolower($filename), "[tmbo]") === false ?
				'[tmbo] '.$filename : $filename;
		$filename = $upload->is_nsfw() == 1 && strpos(strtolower($filename), "[nsfw]") === false ?
				'[nsfw] '.$filename : $filename;

		$fileURL = "https://". $_SERVER['HTTP_HOST'] . $upload->URL();
		$thumbURL = "https://". $_SERVER['HTTP_HOST'] . $upload->thumbURL();
		
		if($upload->URL() == "") continue;
?>
		<item>
			<? if( isset($_GET['gallery']) ) { ?>
				<media:content url="<?= $fileURL ?>" />
				<media:thumbnail url="<?= $thumbURL ?>" />
				<guid isPermaLink="false">tmbo-<?= $upload->id() ?></guid>
			<? } else { ?>
				<title><![CDATA[<?= $filename ?> (uploaded by <?= $upload->uploader()->username() ?>)]]></title>
				<link>https://<?= $_SERVER['HTTP_HOST'] ?><?= Link::upload($upload) ?></link>
				<description><![CDATA[<? // TODO: pretty this up like audio
					if($fileURL != '') { 
						?><img src="<?= $fileURL ?>"/><?
					} else {
						echo "(expired)";
					}
				?>]]></description>
				<pubDate><? echo gmdate( "r", strtotime( $upload->timestamp() ) ) ?></pubDate>			
				<comments><![CDATA[https://<?= $_SERVER['HTTP_HOST'].Link::thread($upload) ?>]]></comments>
			<? } ?>
		</item>
<?
	}
?>
	
	</channel>
</rss>
