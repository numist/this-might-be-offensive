<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
require_once("offensive/assets/conditionalGet.inc");
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
$link = openDbConnection();

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
		<link>http://thismight.be/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><?
		echo gmdate('r', $lastBuildTime);
		?></lastBuildDate>

<?
	$sql = "SELECT offensive_uploads.*, users.username, users.userid
			FROM offensive_uploads
				LEFT JOIN users ON offensive_uploads.userid = users.userid
			WHERE type='topic' AND status='normal'
			ORDER BY timestamp DESC
			LIMIT 200";

	$result = tmbo_query( $sql );

	while( $row = mysql_fetch_assoc( $result ) ) {
	
		$nsfw = $row['nsfw'] == 1 ? "[nsfw]" : "";
	
?>

		<item>
			<title><![CDATA[<? echo $nsfw . $row['filename']?> (started by <? echo $row['username']?>)]]></title>
			<link>http://themaxx.com/offensive/pages/pic.php?id=<? echo $row['id'] ?></link>
			<description><![CDATA[<?
			$sql = "SELECT comment, userid from offensive_comments where fileid = 226019 order by id asc limit 1";
			$res = tmbo_query($sql);
			$ro = mysql_fetch_assoc($res);
			if($ro['userid'] == $row['userid']) {
				echo htmlEncode($ro['comment']);
			}
			?>]]></description>
			<pubDate><? echo date( "r", strtotime( $row['timestamp'] ) ) ?></pubDate>			
			<comments><![CDATA[http://themaxx.com/offensive/page.php?c=comments&fileid=<? echo $row['id'] ?>]]></comments>
		</item>
<?
	}
?>


		
	</channel>
</rss>