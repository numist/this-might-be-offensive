<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

mustLogIn("http");

require_once("offensive/assets/conditionalGet.inc");
require_once("offensive/assets/functions.inc");

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
			<link>http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/</link>
			<description>[ this might be offensive ]</description>
			<lastBuildDate><?= gmdate('r', $lastBuildTime);	?></lastBuildDate>
		<? } 
	$sql = "SELECT offensive_uploads.*, users.username
			FROM offensive_uploads USE KEY (t_t_id)
				LEFT JOIN users ON offensive_uploads.userid = users.userid
			WHERE type='image' AND status='normal'";
	if(isset($_REQUEST['nonsfw'])) $sql .= " AND nsfw = 0";
	if(isset($_REQUEST['notmbo'])) $sql .= " AND tmbo = 0";
	$sql .=	" ORDER BY timestamp DESC
			LIMIT 200";

	$result = tmbo_query( $sql );

	while( $row = mysql_fetch_assoc( $result ) ) {
	
	  // mark tmbo and nsfw files, if they aren't already
		$filename = $row['filename'];
		$filename = $row['tmbo'] == 1 && strpos(strtolower($filename), "[tmbo]") === false ?
				'[tmbo] '.$filename : $filename;
			$filename = $row['nsfw'] == 1 && strpos(strtolower($filename), "[nsfw]") === false ?
				'[nsfw] '.$filename : $filename;

		$fileURL = "http://". $_SERVER['SERVER_NAME'] . getFileURL($row['id'], $row['filename'], $row['timestamp']);
		$thumbURL = "http://". $_SERVER['SERVER_NAME'] . getThumbURL($row['id'], $row['filename'], $row['timestamp']);
?>

		<item>
			<? if( isset($_GET['gallery']) ) { ?>
				<media:content url="<?= $fileURL ?>" />
				<media:thumbnail url="<?= $thumbURL ?>" />
				<guid isPermaLink="false">tmbo-<?= $row['id'] ?></guid>
			<? } else { ?>
				<title><![CDATA[<?= $filename ?> (uploaded by <?= $row['username'] ?>)]]></title>
				<link>http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/pages/pic.php?id=<?= $row['id'] ?></link>
				<description><![CDATA[<? if($fileURL != '') { ?><img src="<?= $fileURL ?>"/><? } else { echo "(expired)"; } ?>]]></description>
				<pubDate><? echo gmdate( "r", strtotime( $row['timestamp'] ) ) ?></pubDate>			
				<comments><![CDATA[http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/?c=comments&fileid=<?= $row['id'] ?>]]></comments>
			<? } ?>
		</item>
<?
	}
?>
	
	</channel>
</rss>
