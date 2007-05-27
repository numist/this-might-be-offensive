<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">


<?php 
	if( file_exists( "./updating.txt" ) ) {
		$updating_now = true;
	}
	
	if( file_exists( "./log.txt" ) ) {
		$last_update = filemtime( "log.txt" );
	} else {
		$last_update = -1;
	}
	
	
	$baseUrl = "http://themaxx.com/";
	
	// a little load balancing. if the time (in seconds) is even
	// use one domain, if odd use another.
//	$thetime = time();
//	if( $thetime % 2 == 0 ) {
//		$baseUrl = "http://rocketsheep.com/";
//	} 

	$baseUrl .= "offensive/";

?>

<html>
<head>
	<title>themaxx.com : [ this might be offensive ]</title>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<meta name="generator" content="BBEdit 6.0.2">
	<meta name="keywords" content="ray hatfield weblog oklahoma city 3d art lightwave movies animation javascript php">
	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>

	<script type="text/javascript" src="http://www.filepile.org/pub/remote_logged_in.php"></script>
	<script type="text/javascript" language="javascript" src="/bitsUtils.js"></script>

	<script type="text/javascript">
		
		function onLoadHandler() {
			setLinkTargets();
			var cookieValue = self.logged_in_fipi ? "true" : "false";
			document.cookie = "logged_in_fipi=" + cookieValue + ";path=/;expires=Monday, 01-Apr-15 23:12:40 GMT";
		}
	
	</script>
	
<style type="text/css">

	/* override definitions in oldskool.css to make main content area wider */	

	#content {
		margin-left:auto;
		margin-right:auto;
		text-align:left;
		line-height: 15px;
		width:771px;
	}
	
	#rightcol {
		width:584px;
		float:left;
		margin-right: auto;
		margin-left: 0px;
	}
	
</style>

</head>

<body bgcolor="#333366" link="#000066" vlink="#000033" onload="onLoadHandler()">

 <?php include( $DOCUMENT_ROOT . "/includes/headerbuttons.txt" );?>
<br>

	<div id="content">
	
		<div id="titleimg"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></div>

	
		<div id="leftcol">
			
			
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">bandwidth isn't free:</div>
					<div class="bluebox">
						<p>help keep this thing running. please make a small donation to help pay for bandwidth.<p>
						<div style="text-align:center">
							<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
								<input type="hidden" name="cmd" value="_xclick">
								<input type="hidden" name="business" value="geek@themaxx.com">
								<input type="hidden" name="item_name" value="bandwidth">
								<input type="hidden" name="no_note" value="1">
								<input type="hidden" name="amount" value="5.00">								
								<input type="hidden" name="currency_code" value="USD">
								<input type="hidden" name="tax" value="0">
								<input type="image" src="https://www.paypal.com/images/x-click-but21.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
							</form>
						</div>
					</div>
				<div class="blackbar"></div>
			</div>
			
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">about box:</div>
					<div class="bluebox">
						<p>the magical auto-updating images page. I did not create or choose or approve these files. It is highly likely that I have not even seen them. Some of it is porn. Some of it is disgusting. Some of it is not suitable for viewing by anyone at all. If you're not comfortable with such things, go elsewhere.</p>
					<!-- <p>If you're want to know more about what this is, and how and why it came into being, the story can be found <a href="/archives/002095.html">here</a>.</p> -->
					</div>
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="heading">archives:</div>
				<div class="bluebox">

						<?php
							$fileList = array();

							$path = "./zips";
							$dir = opendir( $path );
							while( ($file = readdir($dir) ) !== false) {
								if( strpos( $file, ".zip" ) !== false ) {
									$fileList[] = $file;									
								}
							}
							
							sort( $fileList );
							$fileList = array_reverse( $fileList );
							
							foreach( $fileList as $file ) {
								?><a href="zips/<?php echo $file?>"><?php echo $file?></a> (<?php echo number_format(filesize($path . "/" . $file)/1048576, 1)?> Mb)<br/><?php
							}
							
						?>

				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="bluebox" align="center">
					<form name="spawnbox">					
						<?php
							if( $newwindow == "true" ) {
								$checked = "CHECKED";
							} else {
								$checked = "";
							}
						?>
						<input type="checkbox" name="spawn" <?php echo $checked ?> onClick="toggleWindowPref(this.checked)"><a href="#" title="check this box and links will open in new windows." onClick="document.forms['spawnbox'].elements['spawn'].checked = !(document.forms['spawnbox'].elements['spawn'].checked); toggleWindowPref(document.forms['spawnbox'].elements['spawn'].checked); return false">spawn new windows?</a>
					</form>
				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">contact:</div>
					<div class="bluebox">
						<a href="/contact/">email</a><br>
						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
				<div class="blackbar"></div>
			</div>
			
		</div> <!-- end left column -->
		
		<div id="rightcol">
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">
						Last update: <?php if( $last_update > 0) { echo date( "l, F dS Y, h:i:s A T",$last_update); } else { ?>Unknown.<?php }?>
						<?php if( $updating_now ) {
							echo " <span style='color:#ff6600'>(Update in progress)</span>";
						}?>
						<div style="color:#ff6600">It's down. I'll fix it as soon as I can find the time.</div>
					</div>
					<div class="bluebox">
						

<table border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="270">
			<div class="piletitle">pictures</div>
		</td>
		<td width="270">
			<!--<div class="piletitle">top 100</div> -->
		</td>
	</tr>
	<tr>
		<td colspan="2"><img src="graphics/clear.gif" border="0" height="3" width="1"></td>
	</tr>
	
	<tr>
		<td valign="top">
			
		<!-- picpile here. -->

		</td>
		<td valign="top">
			


		<!-- top 100 list here -->
		
		
		
		

		</td>
		<td valign="top">
			<span style="padding-left:50px;" class="usertext"></span>
		</td>
	</tr>
</table>


						
						
					</div>
				<div class="blackbar"></div>
			</div>
		</div>		
		
	</div>
	
<br clear="all">
<div style="text-align:center">
<hr width="300" />
<font face="geneva,monaco,veranda,arial,sans-serif" size="2">
<div style="font-size:10px;font-family:verdana,geneva,monaco,arial,sans-serif">
[ <a href="/" class="textlinks" onmouseover='window.status="[ the blog ]"; return true' onmouseout='window.status=""'>home</a> ]
[ <a href="/art/" class="textlinks" onmouseover='window.status="[ the gallery ]"; return true' onmouseout='window.status=""'>art</a> ]
[ <a href="/past.shtml" class="textlinks" onmouseover='window.status="[ tiny projects ]"; return true' onmouseout='window.status=""'>features</a> ]
[ <a href="/bathroom/" class="textlinks" onmouseover='window.status="[ the bathroom wall project : a public forum ]"; return true' onmouseout='window.status=""'>the bathroom wall</a> ]
[ <a href="/contact/" class="textlinks" onmouseover='window.status="[ contact me ]"; return true' onmouseout='window.status=""'>contact</a> ]<br />

</div>
</font>
<br /><br />
<font face="verdana, geneva, arial" size="1" class="textlinks">contents copyright &copy; 1997-2003 <a href="/contact/" class="textlinks" onmouseover='window.status="[ connect ]"; return true' onmouseout='window.status=""'>Ray Hatfield</a>. All rights reserved.</font>
</div>
<br />

</body>
</html>