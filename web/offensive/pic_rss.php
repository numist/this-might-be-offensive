<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
require_once("offensive/assets/conditionalGet.inc");
require_once("offensive/assets/functions.inc");
// Include, and check we've got a connection to the database.
require_once( 'admin/mysqlConnectionInfo.inc' );
$link = openDbConnection();

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
<rss version="2.0">
	<channel>
		<title>[ this might be offensive ] : images</title>
		<link>http://thismight.be/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><?
			echo gmdate('r', $lastBuildTime);
		?></lastBuildDate>

<?
	$sql = "SELECT offensive_uploads.*, users.username
			FROM offensive_uploads USE KEY (t_t_id)
				LEFT JOIN users ON offensive_uploads.userid = users.userid
			WHERE type='image' AND status='normal'
			ORDER BY timestamp DESC
			LIMIT 200";

	$result = tmbo_query( $sql );

	while( $row = mysql_fetch_assoc( $result ) ) {
	
	  // mark tmbo and nsfw files, if they aren't already
		$filename = $row['filename'];
		$filename = $row['tmbo'] == 1 && strpos(strtolower($filename), "[tmbo]") === false ?
				'[tmbo] '.$filename : $filename;
			$filename = $row['nsfw'] == 1 && strpos(strtolower($filename), "[nsfw]") === false ?
				'[nsfw] '.$filename : $filename;
		
		$time = strtotime( $row['timestamp'] );
		$year = date( "Y", $time );
		$month = date( "m", $time );
		$day = date( "d", $time );
		$extension = substr( $filename, strrpos( $filename, '.' ) );

?>

		<item>
			<title><![CDATA[<?= $filename ?> (uploaded by <?= $row['username'] ?>)]]></title>
			<link>http://thismight.be/offensive/pages/pic.php?id=<? echo $row['id'] ?></link>
			<description><![CDATA[<img src="<?= getFileURL($row['id'], $row['filename'], $row['timestamp']); ?>"/>]]></description>
			<pubDate><? echo gmdate( "r", strtotime( $row['timestamp'] ) ) ?></pubDate>			
			<comments><![CDATA[http://tmbo.org/offensive/?c=comments&fileid=<? echo $row['id'] ?>]]></comments>
		</item>
<?
	}
?>
	
	</channel>
</rss>