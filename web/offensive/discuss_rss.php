<? 
header( "Content-type: text/xml" ); 
set_include_path("..");
require_once("offensive/assets/header.inc");
?>
<rss version="2.0">
	<channel>
		<title>themaxx.com : [ this might be offensive ]</title>
		<link>http://themaxx.com/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><? echo date("r"); ?></lastBuildDate>

<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	$link = openDbConnection();
	
	$sql = "select offensive_uploads.*, users.username
			FROM offensive_uploads
				LEFT JOIN users ON offensive_uploads.userid = users.userid
			WHERE type='image'
			ORDER BY timestamp DESC
			LIMIT 200";

	$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);

	while( $row = mysql_fetch_assoc( $result ) ) {
	
		$nsfw = $row['nsfw'] == 1 ? "[nsfw]" : "";
	
?>

		<item>
			<title><![CDATA[<? echo $nsfw . $row['filename']?> (uploaded by <? echo $row['username']?>)]]></title>
			<link>http://themaxx.com/offensive/pages/pic.php?id=<? echo $row['id'] ?></link>
			<description><![CDATA[<img src="http://images.themaxx.com/mirror.php/offensive/images/picpile/<? echo htmlentities( $row['filename'] )?>"/>]]></description>
			<pubDate><? echo date( "r", strtotime( $row['timestamp'] ) ) ?></pubDate>			
			<comments><![CDATA[http://themaxx.com/offensive/page.php?c=comments&fileid=<? echo $row['id'] ?>]]></comments>
		</item>
<?
	}
?>


		
	</channel>
</rss>


