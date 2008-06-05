<?
	set_include_path("..");
	require_once("offensive/assets/header.inc");
	require_once("offensive/assets/conditionalGet.inc");
	
	$fileList = array();

	$path = "./zips";
	$dir = opendir( $path );
	while( ($file = readdir($dir) ) !== false) {
		if( strpos( $file, ".zip" ) !== false ) {
			$fileList[] = $file;									
		}
	}
	
	rsort( $fileList );
	
	$time = filectime($path."/".$fileList[0]);
	conditionalGet($time);
	
	header('Content-type: text/xml');
	$dateFormat = "r"; 
	
?><rss version="2.0">
	<channel>
		<title>[ this might be offensive ] : archives</title>
		<link>http://<?= $_SERVER['SERVER_NAME'] ?>/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><?= gmdate($dateFormat, $time); ?></lastBuildDate>

<?php
	foreach( $fileList as $file ) {
	
	$url = "http://".$_SERVER['SERVER_NAME']."/offensive/zips/$file";
?>
	<item>
		<title><?php echo $file?></title>
		<link><? echo $url ?></link>
		<description><![CDATA[
			<a href="<?= $url ?>"><b><?= $url ?></b></a> (<? echo byte_format(filesize($path . "/" . $file))?>)<br/><hr/>
			<?
				$manifest = str_replace( ".zip", "_MANIFEST.txt", "$path/$file" );
				if( file_exists( $manifest ) ) {
					$lines = file( $manifest );
					foreach ($lines as $line_num => $line) {
					   echo str_pad( $line_num, 4, "0", STR_PAD_LEFT ) . ": " . htmlspecialchars($line) . "<br />";
					}
				}
			?>
		]]>
		</description>
		<pubDate><? echo date( $dateFormat, filemtime( "$path/$file" ) ) ?></pubDate>
		<enclosure url="<? echo $url ?>" length="<? echo filesize( "$path/$file" )?>" type="application/zip" />			
	</item>
<?php
	}
?>

	</channel>
</rss>