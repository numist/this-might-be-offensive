<?
set_include_path("../..");
require_once("offensive/assets/header.inc");
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

mustLogIn();
?>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
	<title>[ this might be offensive ] : chat</title>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<link rel="stylesheet" type="text/css" href="/offensive/filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>
	<link rel="stylesheet" type="text/css" href="/offensive/nsfw.css.php"/>
	<style type="text/css">

		/* override definitions in oldskool.css to make main content area wider */	

		#content {
			margin-left:auto;
			margin-right:auto;
			text-align:left;
			line-height: 15px;
			width:771px;
		}
	</style>

</head>
<body bgcolor="#333366" link="#000066" vlink="#000033">
	<br>

	<div id="content">
		<div id="titleimg"><a href="../"><img src="../graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></a></div>

		<div class="contentbox">
			<div class="blackbar"></div>
			<div class="heading">welcome to #themaxx on EFnet</div>
			<div class="bluebox">
				<applet code=IRCApplet.class archive="irc.jar,pixx.jar" width=100% height=500>
					<param name="CABINETS" value="irc.cab,securedirc.cab,pixx.cab">
					<param name="language" value="english">
					<param name="pixx:language" value="pixx-english">
							
					<param name="nick" value="<?= $_SESSION['username'] ?>">
					<param name="alternatenick" value="tmb<?= $_SESSION['userid']?>">
					<param name="userid" value="tmb<?= $_SESSION['userid'] ?>">
					<param name="name" value="<?= $_SESSION['username'] ?>">
					<param name="host" value="irc.efnet.net">
					<param name="alternateserver1" value="irc.chowned.org 6667">
					<param name="alternateserver2" value="irc.choopa.net 6667">
					<param name="gui" value="pixx">
					<param name="quitmessage" value="maxx out with your ???? out">
							
					<param name="pixx:highlight" value="true">
					<param name="pixx:highlightnick" value="true">
							
					<param name="coding" value="2">
					
					<param name="command1" value="/join #themaxx">
				</applet>
			</div>
			<div class="heading">not working?  firewalled?  try <a href="http://chat.efnet.org/irc.cgi?chan=%23themaxx">this</a> (thanks netmunky!)</div>
		</div>
	</body>
</html>
