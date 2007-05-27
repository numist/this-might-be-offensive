<?
	$cookiename = $_SESSION['userid'] . "lastpic";
	$lastpic = $_COOKIE[ $cookiename ];
	if( $lastpic ) {
		echo "<div><b><a href=\"pages/pic.php?id=$lastpic\" id=\"pickUp\">click here to pick up where you left off</a></b></div>";
	}
?>
