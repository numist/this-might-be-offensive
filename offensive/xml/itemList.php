<? 
set_include_path("../..");
require_once( 'offensive/assets/header.inc' );

header( "Content-type: text/xml" ); ?>
<itemList version="1.0">

<?
	require_once( 'admin/mysqlConnectionInfo.inc' );
	
	
	function getFileSize( $fpath ) {
		$k = "";
		if( file_exists( $fpath ) ) {
			$size = filesize( $fpath );
			$k = round( ($size/1024) ) . "k";		
		}
		return $k;
	}

	
	if(!isset($link) || !$link) $link = openDbConnection();
	
	$startNum = is_intger( $_REQUEST['start'] ) ? $_REQUEST['start'] : 0;
	$numItems = is_intger( $_REQUEST['num'] ) ? $_REQUEST['num'] : 100;	

	$sql = "select offensive_uploads.*, users.username, offensive_count_cache.*
			FROM offensive_uploads
				LEFT JOIN users ON offensive_uploads.userid = users.userid
				LEFT JOIN offensive_count_cache on threadid=offensive_uploads.id
			WHERE type='image' AND status='normal'
			ORDER BY offensive_uploads.timestamp DESC
			LIMIT $startNum, $numItems";

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

		<item id="<?= $row['id'] ?>">
			<title><![CDATA[<?= $nsfw . $filename ?> (uploaded by <? echo $row['username']?>)]]></title>
			<date><? echo date( "r", strtotime( $row['timestamp'] ) ) ?></date>			
			<imgUrl><?= "http://tmbo.org/offensive/uploads/$year/$month/$day/image/" . rawurlencode( $row['filename'] ) ?></imgUrl>
			<thumbUrl><?= "http://tmbo.org/offensive/uploads/$year/$month/$day/image/thumbs/th" . $row['id'] . $extension ?></thumbUrl>
			<username><![CDATA[<?= $row['username'] ?>]]></username>
			<userId><![CDATA[<?= $row['userid'] ?>]]></userId>
			<fileSize><![CDATA[<?= getFileSize( "../uploads/$year/$month/$day/image/" . $row['filename'] ) ?>]]></fileSize>
			<comments><?= $row['comments']?></comments>
			<good><?= $row['good']?></good>
			<bad><?= $row['bad']?></bad>
			<tmbo><?= $row['tmbo']?></tmbo>
			<repost><?= $row['repost']?></repost>
		</item>
<?
	}
?>


		

</itemList>


