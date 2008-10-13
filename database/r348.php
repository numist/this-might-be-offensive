<?
/* this file is used to populate an initial comment index
 * it may take a long time, and the results might be undefined */

if($argc != 2 || !is_dir($argv[1])) {
        echo "usage: php ".$argv[0]." /path/to/webroot\n";
        exit;
}


class Timer {
        private $time;
        function __construct() {
                $this->time = microtime(true);
        }

        function reset() {
                $now = microtime(true);
                $elapsed = $now - $this->time;
                $this->time = $now;
                return $elapsed;
        }
}

$total = new Timer();

// set up shop.
if(substr($argv[1], -1) == "/") {
        set_include_path(substr($argv[1], 0, -1));
} else {
        set_include_path($argv[1]);
}

require("offensive/assets/header.inc");
require_once('admin/mysqlConnectionInfo.inc');
if(!isset($link) || !$link) $link = openDbConnection();
require_once('offensive/assets/functions.inc');
require_once('Zend/Search/Lucene.php');

login('numist', 'asdf90-=');

// for working on the command line:
ob_end_flush();

//
// OK Go.
//


// this actually does work
error_reporting(E_STRICT);
// because fuck the world.  so:

function shutupshutupshutupshutupshutup($errno, $errstr, $errfile, $errline, $errcontext) {
        return;
}
//set_error_handler('shutupshutupshutupshutupshutup');

/*
 * FIRST, EVERYTHING
 */

echo "
HERE WE GO!
";

$cindexname = get_include_path()."/offensive/data/comments.idx";

echo "comments index will be saved to \"$cindexname\"\n";

if(!file_exists($cindexname)) {
        $cindex = Zend_Search_Lucene::create($cindexname);
} else {
        $cindex = Zend_Search_Lucene::open($cindexname);
}

trigger_error(__LINE__, E_USER_NOTICE);
$uindexname = get_include_path()."/offensive/data/uploads.idx";

echo "uploads index will be saved to \"$uindexname\"\n";

if(!file_exists($uindexname)) {
        $uindex = Zend_Search_Lucene::create($uindexname);
} else {
        $uindex = Zend_Search_Lucene::open($uindexname);
}
trigger_error(__LINE__, E_USER_NOTICE);

$timer = new Timer();

// Get our total for pretty feedback purposes
$sql = "SELECT COUNT(*) AS count FROM offensive_uploads WHERE true";
$result = tmbo_query($sql);
$row = mysql_fetch_assoc($result);
$total_threads = $row['count'];
$current_thread = 0;

flush();

// Get the whole list of threads
$sql = "
SELECT
        offensive_uploads.id AS id,
        users.username,
        offensive_uploads.filename AS filename
FROM
        offensive_uploads, users
WHERE
        users.userid = offensive_uploads.userid
";

$result = tmbo_query($sql);
trigger_error(__LINE__, E_USER_NOTICE);
while($row = mysql_fetch_assoc($result)){

        $current_thread++;
        echo "Working on $current_thread out of $total_threads (".$row['id'].")";
        flush();
        $fileid = $row['id'];
        $filename = $row['filename'];
        $uploader = $row['username'];

        $sql = "
        SELECT
                offensive_comments.id AS commentid,
                users.username,
                offensive_comments.comment
        FROM
                offensive_comments, users
        WHERE
                offensive_comments.fileid = $fileid
		AND users.userid = offensive_comments.userid
        AND
                offensive_comments.comment != ''
        ";

        $thread_result = tmbo_query($sql);

	// and by thread_row, what I really mean is comment :/
	$changed = false;
        while($thread_row = mysql_fetch_assoc($thread_result)) {
          echo ".";
          flush();
          $term = new Zend_Search_Lucene_Index_Term($thread_row['commentid'], 'commentid');
          $docIds = $cindex->termDocs($term);
	  if(count($docIds) > 0) continue;
          
          /*
           *                      value stored?                   indexed?                tokenized?              binary?
           * Keyword              yes                             yes                     no                      no
           * UnIndexed            yes                             no                      no                      no
           * Binary               yes                             no                      no                      yes
           * Text                 yes                             yes                     yes                     no
           * UnStored             no                              yes                     yes                     no
           */
          $doc = new Zend_Search_Lucene_Document();
          @$doc->addField(Zend_Search_Lucene_Field::Keyword('commentid', $thread_row['commentid']));
          @$doc->addField(Zend_Search_Lucene_Field::Text('fileid', $fileid));
          @$doc->addField(Zend_Search_Lucene_Field::Unstored('filename', $filename));
          @$doc->addField(Zend_Search_Lucene_Field::Unstored('comment', $thread_row['comment']));
          @$doc->addField(Zend_Search_Lucene_Field::Unstored('username', $thread_row['username']));
          $cindex->addDocument($doc);
	  $changed = true;
        }
	if($changed)
		$cindex->commit();
        
        //Do the upload:
        $term = new Zend_Search_Lucene_Index_Term($fileid, 'fileid');
        $docIds = $uindex->termDocs($term);
	if(count($docIds) == 0)
	{
        	$doc = new Zend_Search_Lucene_Document();
        	$doc->addField(Zend_Search_Lucene_Field::Keyword('fileid', $fileid));
	        $doc->addField(Zend_Search_Lucene_Field::Unstored('filename', $filename));
	        $doc->addField(Zend_Search_Lucene_Field::Unstored('username', $uploader));
	        $uindex->addDocument($doc);
	        $uindex->commit();
	}
	echo "\n";
        flush();
        mysql_free_result($thread_result);
}

echo "Done.\noptimizing indices... ";
flush();

$cindex->optimize();
$uindex->optimize();

echo "Done (".number_format($timer->reset(), 5)."s)\n\nUpload index size is ".$uindex->count().", ".$uindex->numDocs()." documents.\n\nComment index size is ".$cindex->count().", ".$cindex->numDocs()." documents.\n\n";


?>
