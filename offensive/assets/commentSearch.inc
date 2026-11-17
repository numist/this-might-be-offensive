<?php
/*
 * Comment search, against the PostgreSQL index that admin/commentSearchSync.php
 * keeps (schema: admin/database/search.sql). It only finds comment ids;
 * core_searchcomments() loads the comments from MariaDB.
 *
 * Queries are web-search style, as websearch_to_tsquery() reads them:
 * words (all required), -excluded words, and "or". On top of that a word
 * ending in * matches as a prefix, and a "quoted phrase" matches those words
 * in that order, stopwords included ("i get it"). Prefixes and phrases are
 * always required (or excluded, with -): "or" doesn't apply to them. Words
 * are stemmed (english), so "dogs" finds "dog", and outside phrases
 * stopwords like "the" are ignored.
 */

// What comments are indexed with: english stemming that keeps stopwords
// (admin/database/search.sql). Changing it makes the sync rebuild the index.
const COMMENT_SEARCH_INDEX_CONFIG = "english_nostop";

// A search is cut off after this long and answers 422 instead. Newest-first
// searches are milliseconds; this is for relevance sorting of a common word,
// which has to rank every comment that contains it, and for very deep pages.
const COMMENT_SEARCH_TIMEOUT_MS = 3000;

// No search reads past this many results. Deep offsets cost as much as
// reading every result before them, and nobody pages that far.
const COMMENT_SEARCH_MAX_OFFSET = 10000;

// If the search database stops answering, give up on it this quickly
// rather than holding a PHP worker until the operating system does.
const COMMENT_SEARCH_CONNECT_TIMEOUT = 2;

/* A search_db_* setting from admin/.config, else from the environment
 * (SEARCH_DB_DSN and so on), which is how docker-compose.yml sets them. */
function commentSearchSetting($key, $default = null)
{
	global $config;
	if(isset($config[$key]) && $config[$key] !== "") {
		return $config[$key];
	}
	$env = getenv(strtoupper($key));
	return $env !== false && $env !== "" ? $env : $default;
}

function commentSearchDb()
{
	static $db = null;
	if($db === null) {
		$db = new PDO(commentSearchSetting("search_db_dsn", "pgsql:host=localhost;dbname=tmbo_search"),
		              commentSearchSetting("search_db_user"),
		              commentSearchSetting("search_db_pass"),
		              [
		              	PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
		              	PDO::ATTR_TIMEOUT => COMMENT_SEARCH_CONNECT_TIMEOUT,
		              	// Bind values as literals, so the planner sees the actual
		              	// query words and can tell a rare word from a common one.
		              	PDO::ATTR_EMULATE_PREPARES => true,
		              ]);
	}
	return $db;
}

/* MariaDB's latin1 columns hold UTF-8 bytes, but not every old comment is
 * valid UTF-8, and PostgreSQL refuses text that isn't. Old non-UTF-8 text
 * was almost certainly typed as Windows-1252. */
function commentSearchText($s)
{
	if(!mb_check_encoding($s, "UTF-8")) {
		$s = mb_convert_encoding($s, "UTF-8", "Windows-1252");
	}
	return str_replace("\0", "", $s);
}

/* The tsquery for what the user typed, as text, or null when there's nothing
 * in it to look up (only stopwords, or only exclusions, which would mean
 * reading every comment). */
function commentSearchQuery($q)
{
	$q = commentSearchText($q);
	if(mb_strlen($q) > 300) {
		$q = mb_substr($q, 0, 300);
	}

	// Phrases go through the index's own configuration, so their stopwords
	// count. Up to five, like prefixes; past that, a phrase's words stay in
	// the query as loose words (an excluded one is dropped).
	// Phones type curly quotes (iOS smart punctuation), which count too.
	$q = str_replace(["\u{201C}", "\u{201D}", "\u{201E}", "\u{201F}"], '"', $q);
	$phrases = [];
	$q = preg_replace_callback('/(?<=^|\s)(-?)"([^"]*)"/u', function($m) use (&$phrases) {
		if(trim($m[2]) === "") {
			return " ";
		}
		if(count($phrases) >= 5) {
			return $m[1] === "-" ? " " : " ".$m[2]." ";
		}
		$phrases[] = [$m[1] === "-", $m[2]];
		return " ";
	}, $q);

	// websearch_to_tsquery() has no prefix syntax, so take out word* and add
	// it as word:*. Only letters and digits get through, so it can't carry
	// tsquery operators. Short prefixes match too much to be useful. Past
	// the fifth, a prefix stays in the query as a plain word, so the search
	// doesn't get broader than what was typed.
	$prefixes = [];
	$q = preg_replace_callback('/(?<=^|\s)(-?)([\p{L}\p{N}]{3,})\*(?=\s|$)/u', function($m) use (&$prefixes) {
		if(count($prefixes) >= 5) {
			return $m[1].$m[2];
		}
		$prefixes[] = [$m[1] === "-", $m[2]];
		return " ";
	}, $q);

	$sql = "websearch_to_tsquery('english', ?)";
	$params = [$q];
	foreach($phrases as $p) {
		$sql = "($sql && ".($p[0] ? "!!" : "")."phraseto_tsquery('".COMMENT_SEARCH_INDEX_CONFIG."', ?))";
		$params[] = $p[1];
	}
	foreach($prefixes as $p) {
		$sql = "($sql && ".($p[0] ? "!!" : "")."to_tsquery('english', ? || ':*'))";
		$params[] = $p[1];
	}

	$db = commentSearchDb();
	$st = $db->prepare("SELECT q::text AS q, querytree(q) AS tree FROM (SELECT $sql AS q) s");
	$st->execute($params);
	$row = $st->fetch(PDO::FETCH_ASSOC);
	// querytree() is the part the index can use; "T" means none of it can
	if($row["q"] === "" || $row["tree"] === "T") {
		return null;
	}
	return $row["q"];
}

/* Ids of matching comments, newest first or best match first. Throws 422
 * when the search runs too long. Nothing past COMMENT_SEARCH_MAX_OFFSET. */
function commentSearchIds($tsquery, $order, $offset, $limit, $exclude_userids = [])
{
	$offset = max(0, (int)$offset);
	$limit = min(max(0, (int)$limit), COMMENT_SEARCH_MAX_OFFSET - $offset);
	if($limit <= 0) {
		return [];
	}

	$where = "tsv @@ ?::tsquery";
	$params = [$tsquery];
	$exclude_userids = array_map("intval", $exclude_userids);
	if(count($exclude_userids) > 0) {
		$where .= " AND userid <> ALL('{".implode(",", $exclude_userids)."}'::integer[])";
	}
	if($order == "relevance") {
		$orderby = "ts_rank(tsv, ?::tsquery) DESC, id DESC";
		$params[] = $tsquery;
	} else {
		$orderby = "id DESC";
	}
	$sql = "SELECT id FROM comments WHERE $where ORDER BY $orderby"
	      ." LIMIT $limit OFFSET $offset";

	$db = commentSearchDb();
	$db->beginTransaction();
	try {
		$db->exec("SET LOCAL statement_timeout = ".COMMENT_SEARCH_TIMEOUT_MS);
		$st = $db->prepare($sql);
		$st->execute($params);
		$ids = $st->fetchAll(PDO::FETCH_COLUMN);
		$db->commit();
	} catch(PDOException $e) {
		$db->rollBack();
		if($e->getCode() == "57014") { // query_canceled
			throw new Exception("search too broad", 422);
		}
		throw $e;
	}
	return array_map("intval", $ids);
}
