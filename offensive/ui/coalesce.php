<?

set_include_path("../..");
require_once("offensive/classes/assets.inc");
require_once("offensive/assets/conditionalGet.inc");

ob_start("ob_gzhandler");

assert('array_key_exists("f", $_GET)');
assert('array_key_exists("type", $_GET)');
assert('in_array($_GET["type"], array("js", "css"))');

$ids = explode(":", $_GET["f"]);
$assets = array();
$latest = 0;
foreach($ids as $id) {
	// avoid filesystem walking
	$asset = Asset::assetFor(base_convert($id, 36, 10));
	if($asset->timestamp() > $latest) {
		$latest = $asset->timestamp();
	}
	$assets[] = $asset;
}

$month = 60*60*24*31;
header("Pragma: public");
header("Cache-Control: max-age=$month, public");
header("Expires: ".gmdate("r", time() + $month));
conditionalGet($latest);

if($_GET["type"] == "js") {
	header("Content-type: text/javascript");
} else {
	header("Content-type: text/css");
}

foreach($assets as $asset) {
	if($asset->useminifile()) {
		echo file_get_contents(get_include_path().$asset->minifile());
	} else {
		echo file_get_contents(get_include_path().$asset->file());
	}
	echo "\n";
}	

?>