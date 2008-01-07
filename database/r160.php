<?
/* this script will migrate a live database from the version <= r159
 * to the current version supported in r160+.
 * if you created your database with the schema from r161 or later,
 * you SHOULD NOT run this script */

function report() {
	$string = mysql_error();
	if(strlen($string) > 0) {
		echo $string."\n";
	}
}

if(isset($_SERVER['REMOTE_ADDR'])) die("this upgrade should only be run from the command-line\n");

error_reporting(E_ALL);
set_time_limit(0);
ini_set("memory_limit", "512M");

set_include_path("../web");
require_once("admin/mysqlConnectionInfo.inc");
if(!isset($link) || !$link) {
	$link = openDbConnection();
}

echo "subscriptions: ".array_pop(mysql_fetch_array(mysql_query(
	"SELECT COUNT(*) FROM offensive_subscriptions")))."\n";
$result = mysql_query("SELECT COUNT(*) FROM offensive_bookmarks") or die("no no don't do it");;
echo "bookmarks: ".array_pop(mysql_fetch_array($result))."\n";

// first, clean up the subscriptions table:
$result = mysql_query("SELECT * FROM offensive_subscriptions");
report();
$hits = array();
while(false !== list($id, $userid, $fileid) = mysql_fetch_array($result)) {
	if(array_key_exists("$userid,$fileid", $hits)) {
		mysql_query("DELETE FROM offensive_subscriptions WHERE id = $id");
		report();
	} else {
		$hits["$userid,$fileid"] = 1;
	}
	echo ".";
	ob_flush();
}

echo "\nno lookin' back now\n";
mysql_query("ALTER TABLE offensive_subscriptions ADD commentid INT default NULL");
report();
echo "."; ob_flush();

mysql_query("ALTER TABLE offensive_subscriptions DROP INDEX `fileid_index___added_by_dreamhost`");
report();
echo "."; ob_flush();

mysql_query("ALTER TABLE offensive_subscriptions DROP INDEX `userid`");
report();
echo "."; ob_flush();

mysql_query("ALTER TABLE `offensive_subscriptions` ADD INDEX `u_f_c` ( `userid` , `fileid` , `commentid` )");
report();
echo "."; ob_flush();

mysql_query("ALTER TABLE offensive_subscriptions DROP COLUMN id");
report();
echo "."; ob_flush();

$result = mysql_query("SELECT b.userid, b.fileid, min(b.commentid) as commentid
	FROM offensive_bookmarks b, users u, offensive_uploads up
	WHERE b.userid = u.userid AND
		up.id = b.fileid AND
		b.commentid IS NOT NULL");
report();
echo "."; ob_flush();

while(false !== list($userid, $fileid, $commentid) = mysql_fetch_array($result)) {
	$sql = "UPDATE offensive_subscriptions SET commentid = $commentid WHERE userid = $userid && fileid = $fileid";
	mysql_query($sql);
	report();
	echo "."; ob_flush();
}

mysql_query("DROP TABLE offensive_bookmarks");
report();
echo ".\n"; ob_flush();

echo "subscriptions: ".array_pop(mysql_fetch_array(mysql_query(
        "SELECT COUNT(*) FROM offensive_subscriptions")))."\n";

?>
