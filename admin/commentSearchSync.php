<?php
/* copies comments from MariaDB into the PostgreSQL search index that comment
 * search reads (offensive/assets/commentSearch.inc, admin/database/search.sql).
 *
 * usage: php admin/commentSearchSync.php /path/to/webroot [--rebuild]
 *
 * without --rebuild it copies the comments added since the last run, which is
 * what cron does every minute. on an empty index, or one built with another
 * text search configuration than COMMENT_SEARCH_INDEX_CONFIG, it rebuilds.
 *
 * --rebuild builds a fresh index beside the live one and swaps it in at the
 * end, so search keeps working meanwhile. use it after changing how comments
 * are indexed (the text search configuration, say). a rebuild that is
 * stopped starts over the next time.
 *
 * comments are never edited or deleted, so new ids are all there is to copy.
 * only one copy runs at a time: a run that finds another one going exits. */

const BATCH = 10000;
// ids are assigned when a comment's INSERT starts but become visible when it
// commits, so one can show up behind a higher id the last run already read.
// re-reading the last few hundred comments each run picks those up.
const OVERLAP = 200;
const LOCK_KEY = 149; // pg_advisory_lock key, after the issue that started this

if(php_sapi_name() != "cli") {
	echo "this script can only be run from the command line\n";
	exit(1);
}
$rebuild = $argc == 3 && $argv[2] == "--rebuild";
if($argc < 2 || $argc > 3 || !is_dir($argv[1]) || ($argc == 3 && !$rebuild)) {
	echo "usage: php ".$argv[0]." /path/to/webroot [--rebuild]\n";
	exit(1);
}

set_include_path(rtrim($argv[1], "/"));
// not header.inc: it starts a session and redirects anything that isn't https.
require("admin/mysqlConnectionInfo.inc");
$link = openDbConnection();
require("offensive/assets/commentSearch.inc");

try {
	$pg = commentSearchDb();
} catch(PDOException $e) {
	fwrite(STDERR, "can't reach the comment search database: ".$e->getMessage()."\n");
	exit(1);
}
if(!$pg->query("SELECT pg_try_advisory_lock(".LOCK_KEY.")")->fetchColumn()) {
	if($rebuild) {
		echo "another sync is running; try again when it's done\n";
		exit(1);
	}
	exit(0);
}
$pg->exec(file_get_contents(get_include_path()."/admin/database/search.sql"));
$pg->exec("CREATE TEMPORARY TABLE staging (id integer, userid integer, body text)");

/* the id OVERLAP comments before $id, to start a run from */
function overlapStart($id)
{
	global $link;
	$result = mysqli_query($link, "SELECT id FROM offensive_comments WHERE id <= $id
	                               ORDER BY id DESC LIMIT 1 OFFSET ".OVERLAP);
	$row = $result ? mysqli_fetch_row($result) : null;
	return $row ? (int)$row[0] - 1 : 0;
}

/* copies comments with ids above $after into $table and returns the highest
 * id it read. with $state, keeps sync_state up to date as it goes. */
function copyComments($table, $after, $state, $progress = false)
{
	global $link, $pg;
	$copied = 0;
	$started = microtime(true);
	while(true) {
		$sql = "SELECT id, userid, comment FROM offensive_comments
		        WHERE id > $after AND comment <> '' ORDER BY id LIMIT ".BATCH;
		if(!($result = mysqli_query($link, $sql))) {
			fwrite(STDERR, mysqli_error($link)."\n");
			exit(1);
		}
		$rows = [];
		while($row = mysqli_fetch_row($result)) {
			// COPY text format: tab-separated, with backslash escapes
			$body = strtr(commentSearchText($row[2]), ["\\" => "\\\\", "\t" => "\\t", "\n" => "\\n", "\r" => "\\r"]);
			$rows[] = "$row[0]\t$row[1]\t$body";
			$after = (int)$row[0];
		}
		mysqli_free_result($result);
		if(count($rows) == 0) {
			return $after;
		}

		$pg->beginTransaction();
		$pg->pgsqlCopyFromArray("staging", $rows);
		// a comment that is only stopwords ("the") has nothing to find it by
		$pg->exec("INSERT INTO $table (id, userid, tsv)
		           SELECT id, userid, tsv FROM (
		               SELECT id, userid, to_tsvector('".COMMENT_SEARCH_INDEX_CONFIG."', body) AS tsv FROM staging
		           ) s WHERE length(tsv) > 0
		           ON CONFLICT (id) DO NOTHING");
		$pg->exec("TRUNCATE staging");
		if($state) {
			$pg->exec("UPDATE sync_state SET last_id = GREATEST(last_id, $after)");
		}
		$pg->commit();

		$copied += count($rows);
		if($progress) {
			fprintf(STDERR, "%d comments, up to id %d, %.0f/s\n", $copied, $after, $copied / (microtime(true) - $started));
		}
		if(count($rows) < BATCH) {
			return $after;
		}
	}
}

$state = $pg->query("SELECT last_id, indexed_with FROM sync_state")->fetch(PDO::FETCH_ASSOC);
$last = (int)$state["last_id"];
// An index from before indexed_with existed was built with 'english'.
$built_with = $state["indexed_with"] !== null ? $state["indexed_with"] : ($last > 0 ? "english" : null);
if(!$rebuild && $last > 0 && $built_with === COMMENT_SEARCH_INDEX_CONFIG) {
	copyComments("comments", overlapStart($last), true);
	exit(0);
}

if(!$rebuild && $last > 0) {
	fwrite(STDERR, "the index was built with '$built_with', rebuilding with '".COMMENT_SEARCH_INDEX_CONFIG."'\n");
}

// Load without the index and build it at the end, which is several times
// faster than keeping it up to date row by row.
$pg->exec("DROP TABLE IF EXISTS comments_new");
$pg->exec("CREATE TABLE comments_new (id integer PRIMARY KEY, userid integer NOT NULL, tsv tsvector NOT NULL)");
$pg->exec("ALTER TABLE comments_new ALTER tsv SET STATISTICS 10000"); // see search.sql
$last = copyComments("comments_new", 0, false, true);
fwrite(STDERR, "building the index\n");
$pg->exec("SET maintenance_work_mem = '256MB'");
$pg->exec("CREATE INDEX comments_new_tsv ON comments_new USING gin (tsv) WITH (fastupdate = off)");
// pick up what was posted during the build
$last = copyComments("comments_new", overlapStart($last), false);

$pg->beginTransaction();
$pg->exec("DROP TABLE comments");
$pg->exec("ALTER TABLE comments_new RENAME TO comments");
$pg->exec("ALTER INDEX comments_new_pkey RENAME TO comments_pkey");
$pg->exec("ALTER INDEX comments_new_tsv RENAME TO comments_tsv");
$pg->exec("UPDATE sync_state SET last_id = $last, indexed_with = '".COMMENT_SEARCH_INDEX_CONFIG."'");
$pg->commit();
$pg->exec("ANALYZE comments");
fwrite(STDERR, "done, up to id $last\n");
