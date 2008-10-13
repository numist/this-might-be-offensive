<?

require_once('Zend/Search/Lucene.php');

function getUpload() {
	
}

function postUpload() {
	
}

function indexUpload($id, $filename, $username) {
	$indexname = get_include_path()."/offensive/data/uploads.idx";
	if(!file_exists($indexname)) {
		$index = Zend_Search_Lucene::create($indexname);
	} else {
		$index = Zend_Search_Lucene::open($indexname);
	}
	$doc = new Zend_Search_Lucene_Document();
	$doc->addField(Zend_Search_Lucene_Field::Keyword('fileid', $id));
	$doc->addField(Zend_Search_Lucene_Field::Unstored('contents', $filename));
    $doc->addField(Zend_Search_Lucene_Field::Unstored('username', $username));
	$index->addDocument($doc);
	$index->commit();
	$index->optimize();
	unset($index);
	
	$indexname = get_include_path()."/offensive/data/comments.idx";
	if(!file_exists($indexname)) {
		$index = Zend_Search_Lucene::create($indexname);
	} else {
		$index = Zend_Search_Lucene::open($indexname);
	}
	$index->commit();
	$index->optimize();
	unset($index);
}

function searchUploads($q) {
	// pending search fixes
}

?>