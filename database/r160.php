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

error_reporting(0);

set_include_path("../web");
require_once("admin/mysqlConnectionInfo.inc");
if(!isset($link) || !$link) {
	$link = openDbConnection();
}

echo "subscriptions: ".array_pop(mysql_fetch_array(mysql_query(
	"SELECT COUNT(*) FROM offensive_subscriptions")))."\n";
echo "bookmarks: ".array_pop(mysql_fetch_array(mysql_query(
	"SELECT COUNT(*) FROM offensive_bookmarks") or die("incompatible db version!")))."\n";

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
}

mysql_query("ALTER TABLE offensive_subscriptions ADD commentid INT default NULL");
report();

mysql_query("ALTER TABLE offensive_subscriptions DROP INDEX `fileid_index___added_by_dreamhost`");
report();

mysql_query("ALTER TABLE offensive_subscriptions DROP INDEX `userid`");
report();

mysql_query("ALTER TABLE `offensive_subscriptions` ADD INDEX `u_f_c` ( `userid` , `fileid` , `commentid` )");
report();

mysql_query("ALTER TABLE offensive_subscriptions DROP COLUMN id");
report();

$result = mysql_query("SELECT b.userid, b.fileid, min(b.commentid) as commentid
	FROM offensive_bookmarks b, users u, offensive_uploads up
	WHERE b.userid = u.userid &&
		up.id = b.fileid
	group by fileid;");
report();

while(false !== list($userid, $fileid, $commentid) = mysql_fetch_array($result)) {
	$sql = "UPDATE offensive_subscriptions SET commentid = $commentid WHERE userid = $userid && fileid = $fileid";
	mysql_query($sql);
	report();
}

mysql_query("DROP TABLE offensive_bookmarks");
report();

echo "subscriptions: ".array_pop(mysql_fetch_array(mysql_query(
        "SELECT COUNT(*) FROM offensive_subscriptions")))."\n";

?>
