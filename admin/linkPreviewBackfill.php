<?php
/* fills the link preview cache (offensive/classes/linkPreview.inc) for every
 * link in every comment, so threads nobody has viewed since previews shipped
 * have them too.  links that already have a fresh preview are skipped, so it
 * is safe to stop and run again.
 *
 * usage: php admin/linkPreviewBackfill.php /path/to/webroot [fileid]
 *
 * with a fileid, only that thread's comments are filled.  it fetches BATCH
 * links at a time and waits between batches to stay around RATE per second,
 * and backs off when the provider says it's being asked too often. */

const BATCH = 10;
const RATE = 5;          // fetches per second
const BACKOFF = 60;      // seconds to wait after a batch with a 429

if(php_sapi_name() != "cli") {
	echo "this script can only be run from the command line\n";
	exit(1);
}
if($argc < 2 || $argc > 3 || !is_dir($argv[1]) || ($argc == 3 && !ctype_digit($argv[2]))) {
	echo "usage: php ".$argv[0]." /path/to/webroot [fileid]\n";
	exit(1);
}

set_include_path(rtrim($argv[1], "/"));
// not header.inc: it starts a session and redirects anything that isn't https.
require("admin/mysqlConnectionInfo.inc");
$link = openDbConnection();
require("offensive/classes/linkPreview.inc");

$fileid = $argc == 3 ? (int)$argv[2] : null;
$counts = array();
$links = 0;

function fill($all) {
	global $counts;
	do {
		$started = microtime(true);
		$statuses = LinkPreview::fetch(BATCH);
		foreach($statuses as $status) {
			$counts[$status] = isset($counts[$status]) ? $counts[$status] + 1 : 1;
		}
		if(in_array(429, $statuses)) {
			echo "rate limited, waiting ".BACKOFF."s\n";
			sleep(BACKOFF);
		}
		$wait = BATCH / RATE - (microtime(true) - $started);
		if($wait > 0) usleep((int)($wait * 1000000));
	} while(LinkPreview::queued() >= ($all ? 1 : BATCH));
}

$last = 0;
do {
	$sql = "SELECT id, comment FROM offensive_comments
	        WHERE id > $last AND comment LIKE '%http%'".
	       ($fileid ? " AND fileid = $fileid" : "").
	       " ORDER BY id LIMIT 1000";
	if(!($result = mysqli_query($link, $sql))) {
		echo mysqli_error($link)."\n";
		exit(1);
	}
	$rows = 0;
	while($row = mysqli_fetch_assoc($result)) {
		$rows++;
		$last = $row["id"];
		// the same urls Comment::HTMLcomment() links
		preg_match_all('/http[s]?:\/\/[^\s<>]+/i', $row["comment"], $matches);
		foreach($matches[0] as $url) {
			// queues it if it's a provider's link without a fresh preview
			LinkPreview::get($url);
			$links++;
		}
		if(LinkPreview::queued() >= BATCH) fill(false);
	}
	if($rows) echo "comments up to $last: $links links, fetched ".json_encode($counts)."\n";
} while($rows > 0);
fill(true);

echo "done.  responses by status (0 is no response): ".json_encode($counts)."\n";

?>
