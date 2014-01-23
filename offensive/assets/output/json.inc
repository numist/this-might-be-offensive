<?

function tmbo_json_encode($data, $send_html) {
	if(is_array($data)) {
		foreach($data as $key => $var) {
			if(is_object($var)) {
				if(get_class($var) == "Error") {
					$data[$key] = "ERROR: ".$var->api_data($send_html);
				} else {
					$data[$key] = $var->api_data($send_html);
				}
			}
		}
	} else if(is_object($data)) {
		if(get_class($data) == "Error") {
			$data = "ERROR: ".$data->api_data($send_html);
		} else {
			$data = $data->api_data($send_html);
		}
  } else {
    $data = array('response' => $data);
  }
	return json_encode($data);
}

?>
