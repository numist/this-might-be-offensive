<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 6.0.2">
	
	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<style type="text/css">
		.toobusy {
			color:#999999;
		}
	</style>
</head>


<body bgcolor="#ffffff">


<?php 
	if( file_exists( "./updating.txt" ) ) {
		$updating_now = true;
	}
	
	$last_update = filemtime( "log.txt" );
?>


<table border="0" cellpadding="0" cellspacing="0" width="100%">
	<tr>
		<td>
			<img src="graphics/fp_header.gif" width="286" height="51"/><br/>			
		</td>
		<td align="right">
			<span class="loggedin">This is not <a href="http://www.filepile.org" class="loggedinlink">FilePile</a>.</span><br><br>
			<a href="wtf.php" class="menulink" style="margin-right:4px;">wtf?</a>&nbsp;
			<a href="/" class="menulink" style="margin-right:4px;">themaxx.com</a>&nbsp;
			<a href="/art/" class="menulink" style="margin-right:4px;">art</a>&nbsp;
			<a href="/bathroom/" class="menulink" style="margin-right:4px;">the bathroom wall</a>&nbsp;
			<a href="/contact/" class="menulink" style="margin-right:4px;">contact</a>&nbsp;
		</td>
	</tr>
	<tr>
		<td colspan="2" background="graphics/fp_dash.gif"><img src="graphics/clear.gif" width="740" height="1"></td>
	</tr>
</table>

<br />
<span class="usertext">
	Last update: <?php echo date( "l, F dS Y, h:i:s A T",$last_update) ?>
	<?php if( $updating_now ) {
		echo " <span style='color:#ff6600'>(Update in progress)</span>";
	}?>
</span>
<br />
<br />

<table border="0" cellpadding="0" cellspacing="0" width="340">
	
</table>


<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td>
			<span class="piletitle">top 100 pile</span><br />			
		</td>
		<td>
			<span class="piletitle">pictures pile</span><br />			
		</td>
	</tr>
	<tr>
		<td colspan="2"><img src="graphics/clear.gif" border="0" height="3" width="1"></td>
	</tr>
	<tr>
		<td background="graphics/fp_dash.gif"><img src="graphics/clear.gif" width="340" height="1"></td>
		<td background="graphics/fp_dash.gif"><img src="graphics/clear.gif" width="340" height="1"></td>		
	</tr>
	<tr>
		<td>
			

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
	
	$class = "oddfile";
	$num = 0;
	while ((list ($key, $val) = each ($fileList)) && $num < 100 ) {
		$class = ($class == "oddfile") ? "evenfile" : "oddfile";
		$num++;
		$filesize = filesize($path."/".$val);
		echo '<span style="margin-right:10px;" class="' . $class . '">' . $num . '</span>';
		?><a href="<?php echo $path."/". urlencode($val)?>" title="<?php echo urldecode($val)?> : <?php echo urldecode($filesize)?> bytes" class="<?php echo $class?>"><?php echo urldecode($val)?></a><?php
		if( $filesize == 1680 ) {
			echo "<span class='toobusy'> [ too busy ]</span>";
		}
		?><br><?php
	}
	

?>

		</td>
		<td valign="top">
			

<?php

	$fileList = array();
	$path = "./images/picpile";
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
	
	$class = "oddfile";
	$num = 0;
	while ((list ($key, $val) = each ($fileList)) && $num < 100 ) {
		$class = ($class == "oddfile") ? "evenfile" : "oddfile";
		$num++;
		$filesize = filesize($path."/".$val);
		echo '<span style="margin-right:10px;" class="' . $class . '">' . $num . '</span>';
		?><a href="<?php echo $path."/". urlencode($val)?>" title="<?php echo urldecode($val)?> : <?php echo urldecode($filesize)?> bytes" class="<?php echo $class?>"><?php echo urldecode($val)?></a><?php
		if( $filesize == 1680 ) {
			echo "<span class='toobusy'> [ too busy ]</span>";
		}
		?><br><?php

	}
	

?>

		</td>
		<td valign="top">
			<span style="padding-left:50px;" class="usertext"></span>
		</td>
	</tr>
</table>


</body>
</html>
