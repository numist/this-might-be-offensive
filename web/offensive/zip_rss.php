<?
	header('Content-type: text/xml');
	$dateFormat = "r"; ?><rss version="2.0">
	<channel>
		<title>[ this might be offensive ] : archives</title>
		<link>http://thismight.be/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><? echo date( $dateFormat ); ?></lastBuildDate>


	<item>
		<title>This feed has moved.</title>
		<link>http://zips.tmbo.org/zip_rss.php</link>
		<description><![CDATA[
			This feed has moved. Please update your reader to point to: <a href="http://zips.tmbo.org/zip_rss.php">http://zips.tmbo.org/zip_rss.php</a><br/><hr/>
		]]>
		</description>
	</item>

	</channel>
</rss>
