<?
	// get db pickuplink
	global $me;
	$dbpic = $me->getPref("ipickup");
	
	// get cookie pickuplink
	$cookiename = $me->id()."lastpic";
	if(array_key_exists($cookiename, $_COOKIE) && is_intger($_COOKIE[$cookiename])) {
		$cookiepic = $_COOKIE[$cookiename];
	} else {
		$cookiepic = false;
	}
	
	// output correct pickuplink
	if($dbpic && $dbpic == $cookiepic) {
		?><div><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">click here to pick up where you left off</a></b></div><?
	} else if ($dbpic && $cookiepic){
		?><div><b>pick up where you left off 
			(<a href="pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">on this computer</a> | 
			<a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">on this account</a>)</b></div><?
	} else if ($cookiepic) {
		?><div><b><a href="pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">pick up where you left off on this computer</a></b></div><?
	} else if ($dbpic) {
		?><div><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">pick up where you left off on this account</a></b></div><?
	}
?>