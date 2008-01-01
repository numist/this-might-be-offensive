<?php

	$fileList = array();

	$path = "zips";
	$dir = opendir( $path );
	while( ($file = readdir($dir) ) !== false) {
		if( strpos( $file, ".zip" ) !== false ) {
			$fileList[] = $file;
		}
	}
	
	sort( $fileList );
	$fileList = array_reverse( $fileList );
	
	foreach( $fileList as $file ) {
		?><a href="/offensive/<?php echo $file; ?>"><?php echo $file; ?></a> (<?php echo number_format(filesize($path . "/" . $file)/1048576, 1)?> MB)<br/><?php
	}
	
?>
