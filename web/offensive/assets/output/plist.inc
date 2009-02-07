<?
/* plist.inc
# vim:set syntax=php:
 * converts between plists (as xml strings) and php native data
 *
 * thanks to http://blog.iconara.net/2007/05/08/php-plist-parsing/
 * for the decoding implementation!
 */

/*
 * Intentionally exported functions:
 *      string plist_encode(mixed $val);
 *      mixed plist_decode(string $plist);
 *
 * !!!: other functions in this library do no input error checking and are
 * not intended to be called from external code.
 */

function tmbo_plist_encode($data) {
	if(is_array($data)) {
		foreach($data as $key => $var) {
			if(is_object($var)) {
				if(get_class($var) == "Error") {
					if(!array_key_exists("errors", $data)) $data["errors"] = array();
					$data["errors"][] = "ERROR: ".$var->api_data();
					unset($data[$key]);
				} else {
					$data[$key] = $var->api_data();
				}
			}
		}
	} else if(is_object($data)) {
		if(get_class($data) == "Error") {
			$data = array("errors" => $data->api_data());
		} else {
			$data = $data->api_data();
		}
	}
	format_data($data);
	return plist_encode($data);
}

// EXAMPLE USAGE:
if(false) {
    $arr = array("foo&", "<bar>", "<baz>" => "qux", 
            array(false, 1, 2, 3.14, date("c")));

    // convert the php data into a plist string
    $plist = plist_encode($arr);
    echo $plist."\n\n";

    // convert the plist string into php data
    var_dump(plist_decode($plist));

    // test that the original data and the doubly-converted data match.
    var_dump($arr === plist_decode($plist)); // true
}

/** Exported functions: **/

/* plist_decode:
 * 
 * Returns php data (array, int, float, string, boolean) from a plist
 * string.
 * 
 * Error Conditions:
 *  if an unrecognized/invalid type is found in the plist, null is returned
 *  in place of the unrecognized value since the plist definition has no
 *  concept of null.  A warning is also generated when invalid data is
 *  encountered.
 */
function plist_decode($xmlstr) {
    $plistDocument = new DOMDocument();
    $plistDocument->loadXML($xmlstr);

    $plistNode = $plistDocument->documentElement;

    $root = $plistNode->firstChild;

    // skip any text nodes before the first value node
    while($root->nodeType == XML_TEXT_NODE) {
        $root = $root->nextSibling;
    }

    return plist_parse_value($root);
}

/* plist_encode:
 *
 * Returns a plist string (in xml format) from a php data structure.
 *
 * Error Conditions:
 *  Due to a lack of type support in plists, the following php types are not
 *  supported, and will be replaced by <false /> in the output, as well as
 *  generating a warning:
 *      * resource
 *      * object
 *      * null
 */
function plist_encode($var) {
    // header and footer around:
    $ret = <<<EOH
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
EOH;
    $ret .= "\n".plist_gen_value($var);
    $ret .= "</plist>\n";
    return $ret;
}

/** plist generation functions: **/
function plist_gen_value($var) {
    $valueType = gettype($var);

    $transformerName = "plist_gen_$valueType";

    $ret = "<false />";
    if(is_callable($transformerName)) {
        $ret = call_user_func($transformerName, $var);
    } else {
        trigger_error("unmatched type $valueType, returning false instead", E_USER_WARNING);
    }

    return $ret;
}

function plist_gen_boolean($val) {
    return "<".($val?"true":"false")." />\n";
}

function plist_gen_integer($val) {
    return "<integer>$val</integer>\n";
}

function plist_gen_double($val) {
    return "<real>$val</real>\n";
}

function plist_gen_string($val) {
    // strings might be dates in disguise...
    // this is pretty inexact for now, not easy to make better.
	$time = strtotime($val);
	if($time !== false && date("c", $time) == $val) {
        return plist_gen_date($val);
    } else {
        return "<string>".htmlEscape($val)."</string>\n";
    }
}

function plist_gen_date($val) {
    return "<date>$val</date>\n";
}

/* generation of a plist from an array is an o(2n) operation */
function plist_gen_array($val) {
    // array in php might be array or dictionary in plist format,
    // so determine which first:
    $type = "array";
    foreach($val as $key => $value) {
        if(!is_numeric($key)) {
            $type = "dict";
        }
    }

    $ret = "<$type>\n";
    foreach($val as $key => $value) {
        if($type == "dict") {
            $ret .= "\t<key>".str_replace(array("&", "<", ">"), array("&amp;", "&lt;", "&gt;"), $key)."</key>\n";
        }

        // indent each line of the result, for prettiness.
        $genned = explode("\n", plist_gen_value($value));
        for($i = 0; $i < count($genned) - 1; $i++) {
            $genned[$i] = "\t".$genned[$i];
        }
        $ret .= implode("\n", $genned);
    }
    $ret .= "</$type>\n";

    return $ret;
}

/** plist parsing functions: **/
function plist_parse_value($valueNode) {
    $valueType = $valueNode->nodeName;

    $transformerName = "plist_parse_$valueType";

    if(is_callable($transformerName)) {
        // there is a transformer function for this node type
        return call_user_func($transformerName, $valueNode);
    }

    trigger_error("No transformer found for $valueType", E_USER_WARNING);
    // if no transformer was found
    return null;
}

function plist_parse_integer( $integerNode ) {
    return (int)$integerNode->textContent;
}

function plist_parse_real( $realNode ) {
    return (double)$realNode->textContent;
}

function plist_parse_string( $stringNode ) {
    return $stringNode->textContent;
}

function plist_parse_date( $dateNode ) {
    return $dateNode->textContent;
}

function plist_parse_true( $trueNode ) {
    return true;
}

function plist_parse_false( $trueNode ) {
    return false;
}

function plist_parse_dict( $dictNode ) {
  $dict = array();

  // for each child of this node
  for (
    $node = $dictNode->firstChild;
    $node != null;
    $node = $node->nextSibling
  ) {
    if ( $node->nodeName == "key" ) {
      $key = $node->textContent;

      $valueNode = $node->nextSibling;

      // skip text nodes
      while ( $valueNode->nodeType == XML_TEXT_NODE ) {
        $valueNode = $valueNode->nextSibling;
      }

      // recursively parse the children
      $value = plist_parse_value($valueNode);

      $dict[$key] = $value;
    }
  }

  return $dict;
}

function plist_parse_array( $arrayNode ) {
  $array = array();

  for($node = $arrayNode->firstChild;
      $node != null;
      $node = $node->nextSibling) {
    if ( $node->nodeType == XML_ELEMENT_NODE ) {
      array_push($array, plist_parse_value($node));
    }
  }

  return $array;
}

?>
