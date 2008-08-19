<?

function xml_encode($data) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
	       xml_gen_value($data);
}

function xml_gen_value($data) {
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
		        }
		    }
			$data = xml_gen_array($data);
			break;
		case "resource":
		case "object":
			$data = serialize($data);
			break;
	}
	return "<$type>$data</$type>\n";
}

function xml_gen_array($data, $tabs="\t") {
	$ret = "\n";
	
	$dict = false;
	foreach($data as $key => $val) {
        if(!is_numeric($key)) {
            $dict = true;
        }
    }

	foreach($data as $key => $val) {
		$type = gettype($val);
		
		if($type == "object" || $type == "resource") {
			$val = serialize($val);
		}

		// safety the weapons.
		if($type == "string" && strtotime($val) > 0) {
			$type = "date";
		} else if(is_array($val)) {
			foreach($val as $k => $v) {
				if(!is_numeric($k)) {
					$type = "dict";
				}
			}
		}
		
		$ret .= "$tabs<element id=\"$key\" type=\"$type\">";

		// format our value
		if(is_array($val)) {
			$ret .= xml_gen_array($val, $tabs."\t");
		} else {
			if($type == "string") {
				$ret .= htmlEscape($val);
			} else {
				$ret .= "$val";
			}
		}
		
		// closure!  at last!
		if(substr($ret, -2) == ">\n") $ret .= $tabs;
		$ret .= "</element>\n";
	}
	return $ret;
}

?>