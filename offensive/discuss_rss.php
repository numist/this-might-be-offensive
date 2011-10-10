<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

mustLogIn("http");

require_once("offensive/assets/conditionalGet.inc");
require_once("offensive/assets/classes.inc");
require_once("offensive/assets/core.inc");

$sql = "SELECT offensive_uploads.timestamp
		FROM offensive_uploads USE KEY (t_t_id)
			LEFT JOIN users ON offensive_uploads.userid = users.userid
		WHERE type='topic' AND status='normal'
		ORDER BY timestamp DESC
		LIMIT 1";
$res = tmbo_query($sql);
$row = mysql_fetch_array($res);
$lastBuildDate = array_pop($row);
$lastBuildTime = strtotime($lastBuildDate);
conditionalGet($lastBuildTime);

?>
<rss version="2.0">
	<channel>
		<title>[ this might be offensive ] : discussions</title>
		<link>http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><?= gmdate('r', $lastBuildTime); ?></lastBuildDate>
<?
	$args = $_REQUEST;
	if(!array_key_exists("type", $args)) {
		$args["type"] = "topic";
	}

	$result = core_getuploads($args);

	foreach( $result as $upload ) {
?>

		<item>
			<title><![CDATA[<?= $upload->filename() ?> (started by <?= $upload->uploader()->username() ?>)]]></title>
			<link>http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/?c=comments&fileid=<?= $upload->id() ?></link>
			<description><![CDATA[<?
				if(count($upload->getComments()) > 0) {
					$comments = $upload->getComments();
					echo $comments[0]->HTMLcomment();
				} else {
					echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(whaddya want, they didn't say anything)";
				}
			?>]]></description>
			<pubDate><?= date( "r", strtotime( $upload->timestamp() ) ) ?></pubDate>			
			<comments><![CDATA[http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/page.php?c=comments&fileid=<?= $upload->id() ?>]]></comments>
		</item>
<?
	}
?>


		
	</channel>
</rss>