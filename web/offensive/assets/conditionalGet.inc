<?

function conditionalGet($timestamp) {
	$last_modified = gmdate('D, d M Y H:i:s \G\M\T', $timestamp);
	$etag = '"'.md5($last_modified).'"';
		
	// Send the headers
	header("Last-Modified: $last_modified");
	header("ETag: $etag");
		
	// See if the client has provided the required headers
	$if_modified_since = isset($_SERVER['HTTP_IF_MODIFIED_SINCE']) ?
		$_SERVER['HTTP_IF_MODIFIED_SINCE'] : false;
				
	$if_none_match = isset($_SERVER['HTTP_IF_NONE_MATCH']) ?
		$_SERVER['HTTP_IF_NONE_MATCH'] : false;
			
		
	// is this a conditional request?
	if (!$if_modified_since && !$if_none_match) {
		return;
	}

	// etag
	if ($if_none_match && $if_none_match != $etag) {
		return; // etag is there but doesn't match
	}

	// if-modified-since
	if ($if_modified_since)
		$clienttime = strtotime($if_modified_since);
	else
		$clienttime = 0;
		
	if ($clienttime < $timestamp) {
			return;
	}
	// Nothing has changed since their last request - serve a 304 and exit
	header('HTTP/1.0 304 Not Modified');
	exit;
}

?>