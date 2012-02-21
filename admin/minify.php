<?

if(php_sapi_name()==="cli") {
	set_include_path(".");
} else {
	set_include_path("..");
}

// need yuicompressor
system("which yuicompressor &> /dev/null", $ret);
if($ret != 0) {
	trigger_error("can not minify, no yuicompressor", E_USER_ERROR);
}

require_once("offensive/classes/assets.inc");

$jsSrc = get_include_path().DIRECTORY_SEPARATOR.JS::srcDir();
$cssSrc = get_include_path().DIRECTORY_SEPARATOR.CSS::srcDir();
$minDir = get_include_path().DIRECTORY_SEPARATOR.AssetManager::minDir();
assert('is_dir($jsSrc)');
assert('is_dir($cssSrc)');
assert('is_dir($minDir)');

// generate new ids
$ids = array();
if(!$handle = opendir($jsSrc)) trigger_error("couldn't open $jsSrc", E_USER_ERROR);
while(false !== ($jsfile = readdir($handle))) {
	if($jsfile == "." || $jsfile == "..") continue;
	$ids[] = DIRECTORY_SEPARATOR.JS::srcDir().DIRECTORY_SEPARATOR.$jsfile;
}
if(!$handle = opendir($cssSrc)) trigger_error("couldn't open $cssSrc", E_USER_ERROR);
while(false !== ($cssfile = readdir($handle))) {
	if($cssfile == "." || $cssfile == "..") continue;
	$ids[] = DIRECTORY_SEPARATOR.CSS::srcDir().DIRECTORY_SEPARATOR.$cssfile;
}

// delete contents of AssetManager::mindir()
if(!$handle = opendir($minDir)) trigger_error("couldn't open $minDir", E_USER_ERROR);
while(false !== ($entry = readdir($handle))) {
	if($entry == "." || $entry == "..") continue;
	$path = $minDir.DIRECTORY_SEPARATOR.$entry;
	echo "removing $path\n";
	if(!unlink($path)) trigger_error("couldn't unlink $path", E_USER_ERROR);
}

// for each file in ids, make an asset, generate minifile
foreach($ids as $key => $src) {
	$path = get_include_path().$src;
	$minifile = $minDir.DIRECTORY_SEPARATOR.base_convert($key, 10, 36).".".base_convert(filesize($path), 10, 36);
	echo "minifying $path to $minifile\n";
	system("yuicompressor '$path' > '$minifile'");
	
	// format files for concatenation:
	$minified = "/*! original file: thismight.be$src */\n".file_get_contents($minifile);
	if(!in_array(substr($minified, -1), array("}", ";"))) {
		$minified .= ";";
	}
	file_put_contents($minifile, $minified);
}

// write out ids.inc
$idsfile = "<?return array(";
foreach($ids as $key => $src) {
	$idsfile .= "$key => \"$src\",";
}
$idsfile .= ");?>";
file_put_contents($minDir."/ids.inc", $idsfile);

?>