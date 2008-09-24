<?
	global $me;
	$lastpic = $me->getPref("ipickup");
	if(is_numeric($lastpic)) {
		?><div><b><a href="pages/pic.php?id=<?= $lastpic ?>" id="pickUp">click here to pick up where you left off</a></b></div><?
	}
?>