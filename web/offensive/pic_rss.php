<? header( "Content-type: text/xml" ); ?>
<rss version="2.0">
	<channel>
		<title>tmbo.org : [ this might be offensive ]</title>
		<link>http://tmbo.org/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><? echo date("r"); ?></lastBuildDate>

<?
	require_once( '../admin/mysqlConnectionInfo.php' );
	
	$link = openDbConnection();
	
	$sql = "select offensive_uploads.*, users.username
			FROM offensive_uploads
				LEFT JOIN users ON offensive_uploads.userid = users.userid
			WHERE type='image' AND status='normal'
			ORDER BY timestamp DESC
			LIMIT 200";

	$result = mysql_query( $sql );

	while( $row = mysql_fetch_assoc( $result ) ) {
	
		$nsfw = $row['nsfw'] == 1 ? "[nsfw]" : "";
		
		$time = strtotime( $row['timestamp'] );
		$year = date( "Y", $time );
		$month = date( "m", $time );
		$day = date( "d", $time );
		$filename = $row['filename'];
		$extension = substr( $filename, strrpos( $filename, '.' ) );
	
?>

		<item>
			<title><![CDATA[<? echo $nsfw . $row['filename']?> (uploaded by <? echo $row['username']?>)]]></title>
			<link>http://thismight.be/offensive/pages/pic.php?id=<? echo $row['id'] ?></link>
			<description><![CDATA[<img src="<?= "http://thismight.be/offensive/uploads/$year/$month/$day/image/" . rawurlencode( $row['filename'] ) ?>"/>]]></description>
			<pubDate><? echo date( "r", strtotime( $row['timestamp'] ) ) ?></pubDate>			
			<comments><![CDATA[http://tmbo.org/offensive/?c=comments&fileid=<? echo $row['id'] ?>]]></comments>
		</item>
<?
	}
?>


		
	</channel>
</rss>


