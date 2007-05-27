<?	
	exit;

	require_once( '../admin/mysqlConnectionInfo.php' );
	
	$link = openDbConnection();
	
	$sql = "SELECT id, filename FROM offensive_uploads WHERE userid=46";
	
	$result = mysql_query( $sql );
	
	while( $row = mysql_fetch_assoc( $result ) ) {
		$filename = $row['filename'];
		$id = $row['id'];
		if( file_exists( "./images/picpile/$filename" ) ) {
			$uptime = date( "Y-m-d h:m:s", filemtime ( "./images/picpile/$filename" ));
			$sql = "UPDATE offensive_uploads SET timestamp = '$uptime' WHERE id=$id LIMIT 1";
			echo $sql;
			mysql_query( $sql );
		}
		else {
			echo "not found: $filename";
		}
		echo "\n<br/>";
	}

?>