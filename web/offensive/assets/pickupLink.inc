<?
	$cookiename = $_SESSION['userid'] . "lastpic";
	$lastpic = array_key_exists($cookiename, $_COOKIE) ? $_COOKIE[ $cookiename ] : null;
	if(is_numeric($lastpic)) {
		echo "<div><b><a href=\"pages/pic.php?id=$lastpic\" id=\"pickUp\">click here to pick up where you left off</a></b></div>";
	}
?>