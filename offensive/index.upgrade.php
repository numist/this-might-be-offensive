<?
	header("HTTP/1.0 502 Bad Gateway");
	header('Content-type: text/html');
	
	if(!defined("TMBO")) {
		set_include_path("..");
	}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title>themaxx.com : this page intentionally left blank.</title>
	<meta name="generator" content="BBEdit 7.0.3">
	
	<style type="text/css">
		p {
			font-family:verdana;
			font-size:11px;
			color:#333366;
		}
		
		a {
			color:#333366;
		}
		
		.copyright {
			font-size:10px;
		}
	</style>
	<? include_once("analytics.inc"); ?>
</head>


<body bgcolor="#ffffff">

	<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
		<tr>
			<td valign="center" height="100%" align="center">
	
				<p>[ upgrade in progress ]</p>
	
			</td>
		</tr>
	</table>
	
<? include 'includes/footer.txt' ?>

</body>
</html>