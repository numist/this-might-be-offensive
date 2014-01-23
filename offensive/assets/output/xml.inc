<?

function tmbo_xml_encode($data) {
	format_data($data);
	return xml_encode($data);
}

function xml_encode($data) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
	       xml_gen_value($data);
}

function xml_gettype($data) {
	$type = gettype($data);
	switch($type) {
		case "string":
			if(strtotime($data) > 0) {
				$type = "date";
			}
			break;
		case "array":
			foreach($data as $key => $val) {
		        if(!is_numeric($key)) {
		            $type = "dict";
					break;
		        }
		    }
			break;
		case "object":
			$type = strtolower(get_class($data));
			break;
	}
	return $type;	
}

function xml_getdata($data, $tabs="\t") {
	$type = gettype($data);
	switch($type) {
		case "array":
			$data = xml_gen_array($data);
			break;
		case "boolean":
			$data = $data ? "true" : "false";
			break;
		case "resource":
			$data = serialize($data);
			break;
		case "object":
			$data = $data->api_data();
			format_data($data);
			if(is_array($data)) {
				$data = xml_gen_array($data, "$tabs\t");
			} else {
				$data = xml_getdata($data);
			}
			break;
	}
	return $data;
}

function xml_gen_value($data) {
	$type = xml_gettype($data);
	$data = xml_getdata($data);
	return "<$type>$data</$type>\n";
}

function xml_gen_array($data, $tabs="\t") {
	$ret = "\n";
	
	$dict = false;
	foreach($data as $key => $val) {
        if(!is_numeric($key)) {
            $dict = true;
			break;
        }
    }

	foreach($data as $key => $val) {
		$type = xml_gettype($val);
		$dat = xml_getdata($val, $tabs);
		if($dict) {
			$ret .= "$tabs<$key type=\"$type\">\n";
		} else {
			$ret .= "$tabs<$type>\n";
		}
		$ret .= "$tabs\t$dat\n";
		if($dict) {
			$ret .= "$tabs</$key>\n";
		} else {
			$ret .= "$tabs</$type>\n";
		}
	}
	return $ret;
}

?>
