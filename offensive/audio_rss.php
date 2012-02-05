<?
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
require_once("offensive/assets/id3.inc");

$sql = "SELECT offensive_uploads.timestamp
		FROM offensive_uploads USE KEY (t_t_id)
			LEFT JOIN users ON offensive_uploads.userid = users.userid
		WHERE type='audio' AND status='normal'
 		ORDER BY timestamp DESC
		LIMIT 1";
$res = tmbo_query($sql);
$row = mysql_fetch_array($res);
$lastBuildDate = array_pop($row);
$lastBuildTime = strtotime($lastBuildDate);
conditionalGet($lastBuildTime);

header( "Content-type: text/xml" ); 
?>
<rss version="2.0"
	xmlns:itunes="http://www.itunes.com/DTDs/Podcast-1.0.dtd" 	
>
	<channel>
		<title>[ this might be offensive ] : audio</title>
		<link>https://<?= $_SERVER['SERVER_NAME'] ?>/offensive/</link>
		<description>[ this might be offensive ]</description>
		<lastBuildDate><? echo date("r"); ?></lastBuildDate>

<?	
	$args = $_REQUEST;
	if(!array_key_exists("type", $args)) {
		$args["type"] = "audio";
	}

	$result = core_getuploads($args);

	foreach( $result as $upload ) {
		// mark tmbo and nsfw files, if they aren't already
		$filename = $upload->filename();
		$filename = $upload->is_tmbo() == 1 && strpos(strtolower($filename), "[tmbo]") === false ?
				'[tmbo] '.$filename : $filename;
		$filename = $upload->is_nsfw() == 1 && strpos(strtolower($filename), "[nsfw]") === false ?
				'[nsfw] '.$filename : $filename;

		$server = "https://". $_SERVER['SERVER_NAME'];
		$fileURL = $server . $upload->URL();
		$thumbURL = $server . $upload->thumbURL();
		
		if(!file_exists($upload->file())) continue;
?>
		<item>
			<title><![CDATA[<?= $filename ?> (uploaded by <?= $upload->uploader()->username() ?>)]]></title>
			<link>https://<?= $_SERVER['SERVER_NAME'] ?><?= Link::upload($upload) ?></link>
			<enclosure url="<?= $fileURL ?>" length="<?= filesize( $upload->file() ) ?>" type="audio/mpeg"/>
			<description><?
			
				ob_start();
				
				$args = "mp3=".urlencode($server.$upload->URL())."&amp;".
						"width=500&amp;".
						"showvolume=1&amp;".
						"showloading=always&amp;".
						"buttonwidth=25&amp;".
						"sliderwidth=15&amp;".
						"volumewidth=36&amp;".
						"volumeheight=8&amp;".
						"loadingcolor=9d9d9d&amp;".
						"sliderovercolor=9999ff&amp;".
						"buttonovercolor=9999ff&amp;".
						"autoload=1";
				
				$fp = fopen($upload->file(), 'r');
				$id3 = new getid3_id3v2($fp, $info);
				
				if(array_key_exists('id3v2', $info) && array_key_exists('comments', $info['id3v2'])) {
					?><table><tr><td><?
					if(file_exists(dirname($upload->file())."/thumbs/th".$upload->id())) {
						?><img src="<?= $server.dirname($upload->URL()) ?>/thumbs/th<?= $upload->id() ?>"></td><td><?
					}
						
					$tags = $info['id3v2']['comments'];
						
					if(array_key_exists('title', $tags)) { ?>
					<span style="color:#666666">Title: <? 
						echo trim($tags['title'][0]); 
						if(array_key_exists('tracknum', $tags)) {
							echo "(track ".(int)trim($tags['tracknum'][0]);
							if(array_key_exists('totaltracks', $tags)) {
								echo " of ".(int)trim($tags['totaltracks'][0]);
							}
							echo ")";
						}
						?>
					</span><br />
					<? }
						
					if(array_key_exists('artist', $tags)) { ?>
					<span style="color:#666666">By: <?= trim($tags['artist'][0]); ?></span><br />
					<? }
						
					if(array_key_exists('album', $tags)) { ?>
					<span style="color:#666666">Album: <?= trim($tags['album'][0]); ?></span><br /><br />
					<? }
					?></td></tr></table><?
						
				} else {
					echo $upload->htmlFilename();
				}
					
			?>
			
			<object type="application/x-shockwave-flash" data="<?= $server ?>/offensive/ui/player_mp3_maxi.swf" width="500" height="20">
			    <param name="movie" value="<?= $server ?>/offensive/ui/player_mp3_maxi.swf" />
			    <param name="bgcolor" value="#ffffff" />
			    <param name="FlashVars" value="<?= $args ?>" />
			</object>
			<?
			
			$string = ob_get_contents();
			ob_end_clean();
			echo str_replace(array("<", ">"), array("&lt;", "&gt;"), $string);
			
			?>
			</description>
			<pubDate><? echo date( "r", strtotime( $upload->timestamp() ) ) ?></pubDate>			
			<comments><![CDATA[https://<?= $_SERVER['SERVER_NAME'].Link::thread($upload) ?>]]></comments>
		</item>
<?
	}
?>

	</channel>
</rss>