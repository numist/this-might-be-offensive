<?

function tmbo_json_encode($data) {
	if(is_array($data)) {
		foreach($data as $key => $var) {
			if(is_object($var)) {
				if(get_class($var) == "Error") {
					$data[$key] = "ERROR: ".$var->api_data();
				} else {
					$data[$key] = $var->api_data();
				}
			}
		}
	} else if(is_object($data)) {
		if(get_class($data) == "Error") {
			$data = "ERROR: ".$data->api_data();
		} else {
			$data = $data->api_data();
		}
	}
	return json_encode($data);
}

?>