<? 
	/* this is probably something we shouldn't be exposing so easily */
	exit; 
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
	<title>Untitled</title>
	<meta name="generator" content="BBEdit 8.0">
</head>
<body>

<? 
	require_once( "activationFunctions.inc" );
	echo activationMessageFor( $_REQUEST['userid'], $_REQUEST['email'] );
?>

</body>
</html>
