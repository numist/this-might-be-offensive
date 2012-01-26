<?
	// get db pickuplink
	global $activeTab;
	
	switch($activeTab) {
		case "yearbook":
			$prefname = "ypickup";
			$cookiename = me()->id()."lastavatar";
			break;
		case "audio":
			$prefname = "apickup";
			$cookiename = me()->id()."lasttrack";
			break;
		default:
			$prefname = "ipickup";
			$cookiename = me()->id()."lastpic";
	}
	
	// get db pickuplink
	$dbpic = me()->getPref($prefname);

	// get cookie pickuplink
	if(array_key_exists($cookiename, $_COOKIE) && is_intger($_COOKIE[$cookiename])) {
		$cookiepic = $_COOKIE[$cookiename];
	} else {
		$cookiepic = 0;
	}
	
	// output correct pickuplink
	if($dbpic != null && $dbpic == $cookiepic) {
		?><div id="pickuplink" fileid="<?= $dbpic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">click here to pick up where you left off</a></b></div><?
	} else if ($dbpic && $cookiepic){
		?><div id="pickuplink"><b>pick up where you left off 
			(<a href="pages/pic.php?id=<?= $cookiepic ?>" fileid="<?= $cookiepic ?>" id="pickUp">on this computer</a> | 
			<a href="pages/pic.php?id=<?= $dbpic ?>" fileid="<?= $dbpic ?>" id="pickUp">on this account</a>)</b></div><?
	} else if ($cookiepic != 0) {
		trigger_error("this shouldn't ever happen anymore...", E_USER_NOTICE);
		?><div id="pickuplink" fileid="<?= $cookiepic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">pick up where you left off on this computer</a></b></div><?
	} else if ($dbpic) {
		?><div id="pickuplink" fileid="<?= $dbpic ?>" style="display: none;"><b><a href="pages/pic.php?id=<?= $dbpic ?>" id="pickUp">pick up where you left off on this account</a></b></div><?
	}
?>