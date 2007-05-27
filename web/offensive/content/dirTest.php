<?

	function ensureDirExists( $path ) {
	
		$basePath = "/home/.chippy/fleece/themaxx.com/offensive/images/";
	
		echo $filePath;
		$curPath = $basePath;
		foreach( explode( '/', $path ) as $dir ) {
			$curPath .= "/$dir";
			if( ! file_exists( $curPath ) ) {			
				echo "creating $curPath";
				mkdir( $curPath );
			}

		}

	}
	
	$curdate = time();
	$year = date( "Y", $curdate );
	$month = date( "m", $curdate );
	$day = date( "d", $curdate );
	
	ensureDirExists( "$year/$month/$day" );

?>

