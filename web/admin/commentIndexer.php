<?

// before we set our tmbo environment, we're going to need to include the php5 class wrapper
require_once("xapian.php");

// set up the normal TMBO environment
set_include_path("/home/thismightbe/sites/tmbo");
require_once( 'offensive/assets/header.inc' );
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

// do not allow this file to run other than on the command line
if (php_sapi_name() != "cli") {
    trigger_error("this script can only be run from the command line (preferably by cron)", E_USER_ERROR);
    exit(1);
}

/*
 * GOD DAMN IT ALWAYS FLUSH ALWAYS ALWAYS
 */
@ini_set('implicit_flush', 1);
while(ob_get_level() > 0) { ob_end_flush(); }
ob_implicit_flush(1);


/*
 * This is the only file that writes to the data/comments.db Xapian index.
 */

$indexfile = get_include_path()."/offensive/data/comments.db";
$lastcfile = get_include_path()."/offensive/data/comments.db.pickup";
$lockfile = get_include_path()."/offensive/data/comments.db.lock";
$stepsize = 1000;

// aquire lock.  try a couple times in a 5-second window
$starttime = microtime(true);
$fp = fopen($lockfile, "w+");
while(microtime(true) < $starttime + 5 && false == ($locked = flock($fp, LOCK_EX | LOCK_NB))) {
	usleep(round(rand(100, 400) * 1000));
}
if(!$locked) {
	trigger_error("another process already holds this lock ($lockfile)", E_USER_ERROR);
	exit(2);
}

// check for inconsistent data state
if(!file_exists($indexfile) || !file_exists($lastcfile)) {
	if(file_exists($lastcfile)) {
		rmr($lastcfile);
	}
	if(file_exists($indexfile)) {
		rmr($indexfile);
	}
}

// pick up where we left off
if(file_exists($lastcfile)) {
	$lastc = trim(file_get_contents($lastcfile));
	// if the file is corrupted, we have to start from scratch
	if(!is_numeric($lastc) || strpos($lastc, ".") !== false) {
		// throw out the corrupt file and the index
		rmr($lastcfile);
		// XXX: usually I don't find myself saying this, but this would be a great time to have a goto instruction
		if(!file_exists($indexfile))
			rmr($indexfile);
	}
}
if(!file_exists($lastcfile)) {
	// if we've never left off before, just start at 1.  it'll figure itself out.
	$lastc = 0;
}

// ok, here we go
try {
	// set up the Xapian environment
    $database = new XapianWritableDatabase($indexfile, Xapian::DB_CREATE_OR_OPEN);
    $indexer = new XapianTermGenerator();
    $stemmer = new XapianStem("english");
    $indexer->set_stemmer($stemmer);
	$comments = 0;
	
	while(true) {
		// get a batch of comments
		$sql = "SELECT * FROM offensive_comments WHERE id != $lastc AND id > $lastc AND id <= ".($lastc + $stepsize)
		       ." ORDER BY id ASC";
		$res = tmbo_query($sql);
		
		while($row = mysql_fetch_assoc($res)) {
			// update the last comment we've seen
			$lastc = $row["id"];
			
			// skip comments with no comment (votes)
			if(trim($row["comment"]) == '') continue;
			
			$comments++;
			index_comment($database, $indexer, $row);
			file_put_contents($lastcfile, $lastc);
		}
		
		// escape when we're all caught up and our query returns 0 results
		$sql = "SELECT MIN(id) FROM offensive_comments WHERE id > $lastc";
		$next = getsingle($sql);
		if($next == $lastc) {
			trigger_error("FUCKING WINDOWS 98, GET BILL GATES IN HERE", E_USER_ERROR);
		}
//		echo "$lastc $comments $next\n";
		if($next == null) break;
		// save some time if there are many unused comment ids
		$lastc = $next - 1;
	}

	// force the Xapian database to be garbage collected now to fire its desctructor.
	$database = null;
} catch (Exception $e) {
    print $e->getMessage() . "\n";
    exit(1);
}

flock($fp, LOCK_UN);
fclose($fp);



/* ******************
 * included functions
 */

function index_comment($database, $indexer, $row) {
	$doc = new XapianDocument();
	$doc->set_data($row["id"]);
//	$doc->set_data($row["comment"]);
//	$doc->add_value(1, (string)$row["id"]);

    $indexer->set_document($doc);
    $indexer->index_text($row["comment"]);

    // Add the document to the database.
    $database->add_document($doc);
}

function getsingle($sql) {
	$res = @tmbo_query($sql);
	return mysql_result($res, 0);
}

function rmr($dir)
{
	if(!is_dir($dir)) {
		unlink($dir);
		return;
	}
	
	$dh=opendir($dir);
	
	while($file=readdir($dh))
	{
    	if($file != "." && $file != "..")
    	{
      		rmr($dir."/".$file);
    	}
  	}
  	closedir($dh);
  	rmdir($dir);
  	return;
}

?>