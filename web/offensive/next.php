<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 6.0.2">
	
	
</head>


<body bgcolor="#ffffff">

<?php

	$fileList = array();

	$path = "./images/top100";
	$dir = opendir( $path );
	while( ($file = readdir($dir) ) !== false) {
	
		if ($file != "." and $file != "..") {
	
			$fileTime=filemtime($path."/".$file);
			$entryExists = true;
			while( $entryExists ) {
				$fileTime++;
				if( !$fileList[$fileTime] ) {
					$entryExists = false;
				}
			}
			$fileList[$fileTime] = $file;
	
		}
	}
	
	krsort($fileList);
	
	echo "-- $fileList[2] --";
	
	$class = "oddfile";
	$num = 0;
	while ((list ($key, $val) = each ($fileList)) && $num < 100 ) {
		$class = ($class == "oddfile") ? "evenfile" : "oddfile";
		$num++;
		$filesize = filesize($path."/".$val);
		$filename = urldecode($val);
		echo '<span style="margin-right:10px;" class="' . $class . '">' . $num . '</span>';
		?><a href="<?php echo $path."/". urlencode($val)?>" title="<?php echo urldecode($val)?> : <?php echo urldecode($filesize)?> bytes" class="<?php echo $class?>"><?php echo substr(urldecode($val), 0, 30)?></a><?php
		if( $filesize == 1680 ) {
			echo "<span class='toobusy'> [ too busy ]</span>";
		}
		?><br><?php
	}
	

?>


</body>
</html>
