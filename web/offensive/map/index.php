<?
	session_start();
	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ../?c=mustLogIn" );
	}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 7.0.3">
	
	
</head>


<body bgcolor="#ffffff">
	
	<?
		if( $_REQUEST['p'] ) {
	?>
			<div style="margin:18px;">Thanks. Your location will be reflected in the next update. (Under 2 minutes.)</div>
	<?
		}
		else {
	?>
			<div style="margin:18px;"><a href="maxxerloc.php">Click here</a> to set your location. <a href="../?c=map">Click here</a> for the minimap page.</div>
	<?		
		}
	?>

	<div>
		<img src="comp.jpg" width="5000" height="3000"/>
	</div>

</body>
</html>
