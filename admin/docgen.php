<?

//$tmboroot = "~/sites/tmbo/";
$tmboroot = "";

$get_examples = false;

	//$token = "rsk48uhhv328w4gq5jjlvubybsxdycqd";
	$token = "";

	//$api = "http://dev.thismight.be/offensive/api.php/";
	$api = "";

/*
 ******************************************************************************
 * IGNORE ME
 ******************************************************************************
 */

if(!is_dir($tmboroot."offensive")) {
	trigger_error("tmboroot is not correct", E_USER_ERROR);
}
if($get_examples) {
	if(strlen($token) != 32) {
		trigger_error("token required to get example output", E_USER_ERROR);
	}
	if(!strlen($api)) {
		trigger_error("api link required to get example output", E_USER_ERROR);
	}
}

/*
 ******************************************************************************
 * HTTP codes
 ******************************************************************************
 */

$codes = array(
	"200" => "<em>OK</em> - Call completed as dialled.",
	"304" => "<em>Not Modified</em> - If a Conditional GET is used, this indicates that there are no changes to report.",
	"400" => "<em>Bad Request</em> - The call is erroneous. The body will contain debugging information.",
	"401" => "<em>Unauthorized</em> - Login failure. Either your credentials weren't valid, or you did not provide any.",
	"403" => "<em>Forbidden</em> - Refusal. You have attempted to log in too many times in a rolling 30 minute period and you are blocked from any more attempts. Wait and try again later.",
	"404" => "<em>Not Found</em> - The function being requested does not exist.",
	"500" => "<em>Internal Server Error</em> - kaboom. Please report if reproducible.",
	"502" => "<em>Bad Gateway</em> - TMBO is down for maintenance. Please wait a few minutes and try your call again."
);

/*
 ******************************************************************************
 * argument types
 ******************************************************************************
 */

$ptypes = array(
	"string" => "UTF-8 string.",
	"integer" => "Whole number.",
	"date" => "Any format parseable by <a href=\"http://php.net/strtotime\">strtotime</a> is appropriate. The server time zone is America/Los_Angeles.",
	"limit" => "Format: \"%d,%d\" or \"%d\". Syntax is the same as <a href=\"http://dev.mysql.com/doc/refman/5.0/en/select.html\">LIMIT in SQL</a>. Default and largest accepted set size is 200.",
	"float" => "Double precision floating point number."
);

/*
 ******************************************************************************
 * return types
 ******************************************************************************
 */

$rtypes = array(
	"json" => "A <a href=\"http://json.org/\">JSON</a> structure, especially suitable for use by Javascript clients.",
	"plist" => "An XML-formatted <a href=\"http://en.wikipedia.org/wiki/Property_list\">Property List</a> structure, especially suitable for use by Cocoa clients.",
	"php" => "A <a href=\"http://us3.php.net/serialize\">serialized</a> PHP structure, especially suitable for use by PHP clients.",
	"xml" => "A basic XML-formatted structure, suitable for use by no one."
);

/*
 ******************************************************************************
 * Parse phpdoc from api.php for function documentation.
 ******************************************************************************
 */

$lines = file($tmboroot."offensive/api.php") or die();
$in_tag = false;
$methods = array();
$method = "";
$param = "";
$value = "";

$linenum = 0;
foreach($lines as $line) {
	$linenum++;
	if(trim($line) == "/**") {
		$in_tag = true;
	}
	
	if(trim($line) == "*/") {
		$in_tag = false;
		$method = "";
		$param = "";
	}

	if($in_tag) {
		if(strpos(trim($line), "*") !== 0) {
			continue;
		}
		$nline = trim(substr($line, strpos($line, "*") + 1));
		$words = explode(" ", $nline);
		switch($words[0]) {
			case "@method":
				$method = $words[1];
				assert('!array_key_exists($method, $methods)') or trigger_error("method $method is being defined another time at $linenum", E_USER_WARNING);
				$methods[$method] = array("params" => array(),
				                          "examples" => array(),
				                          "see" => array(),
				                          "desc" => "");
				break;
			case "@param":
				assert('$method != ""') or trigger_error("no method defined before line $linenum", E_USER_ERROR);

				array_shift($words);
				$param = array_shift($words);
				$type = array_shift($words);
				$req = array_shift($words);
				$values = "";

				assert('count($words)') or trigger_error("no parameter description at line $linenum", E_USER_ERROR);
				if(strpos($words[0], "{") === 0) {
					while(substr(trim($words[0]), -1) != "}") {
						$values .= array_shift($words);
					}
					$values .= array_shift($words);
				}

				$desc = implode(" ", $words);

				$methods[$method]["params"][$param] = array("type" => $type,
				                                            "req" => $req,
				                                            "values" => $values,
				                                            "desc" => $desc);
				$value =& $methods[$method]["params"][$param]["desc"];

				break;
			case "@example":
				assert('$method != ""') or trigger_error("no method defined before line $linenum", E_USER_ERROR);
				$methods[$method]["examples"][] = count($words) >= 2 ? $words[1] : "";
				break;
			case "@see":
				assert('$method != ""') or trigger_error("no method defined before line $linenum", E_USER_ERROR);
				assert('count($words) >= 2') or trigger_error("no target for @see at line $linenum", E_USER_ERROR);
				$methods[$method]["see"][] = $words[1];
				break;
			case "@return":
				assert('$method != ""') or trigger_error("no method defined before line $linenum", E_USER_ERROR);
				array_shift($words);
				assert('count($words)') or trigger_error("no return description at line $linenum", E_USER_ERROR);
				$methods[$method]["return"] = implode(" ", $words);
				break;
			default:
				assert('$method != ""') or trigger_error("no method defined before line $linenum", E_USER_ERROR);
				if($value && $param) {
					$value .= "\n".$nline;
				} else {
					$methods[$method]["desc"] .= "\n".$nline;
				}
				break;
		}
	}
}

/*
 ******************************************************************************
 * Example generation
 ******************************************************************************
 */
if($get_examples) {
	$docsdir = $tmboroot."offensive/docs";
	if(!is_dir($docsdir)) {
		if(!mkdir($docsdir)) {
			trigger_error("could not create docs directory $docsdir", E_USER_ERROR);
		}
	} else {
		$contents = scandir($docsdir);
		foreach($contents as $filename) {
			if(substr($filename, 0, 1) == ".") {
				continue;
			}
			unlink("$docsdir/$filename");
		}
	}
	
	foreach($methods as $method => $data) {
		foreach($data["examples"] as $example) {
			foreach($rtypes as $type => $desc) {
				$call = $api.$method.".".$type."?".$example."&token=".$token;
				// XXX: instrument calls for time
				$ex = file_get_contents($call) or trigger_error("call $call was not valid?", E_USER_ERROR);
				file_put_contents("$docsdir/$method.$type?$example", $ex) or die("can't write example to file?");
			}
		}
	}
}

?>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8 "/> 
	<title>TMBO API Specification</title>
</head>
<body>

<h2>TMBO API Specification</h2>
<p>This document describes functions, their arguments, return values, error codes, and other information used to interact with the TMBO server at thismight.be.</p>

<hr/>
<a name="Authentication">
<h3>Authentication</h3>
<p>There are three methods of authentication:</p>

<ul>
	<li>Tokens - A token generated by the <a href="#login">login method</a> can be used as an argument to any call except <a href="#logout">logout</a>. This is the preferred method of authentication.</li>
	<li>Session cookie - A client that supports cookies may log in with the <a href="#login">login method</a>, which will attempt to set a session cookie for short-term authentication if a token is not requested.</li>
	<li>Basic access authentication - The username and password is provided for every call using <a href="http://en.wikipedia.org/wiki/Basic_access_authentication">HTTP basic access authentication</a>.</li>
<!--	<li>rememberme cookie - Setting the rememeberme option to the <a href="#login">login method</a> will cause it to set a rememberme cookie that can be used for long-term authentication.</li> -->
</ul>


<hr/>
<h3>Parameter Types</h3>
<p>The API supports <?= count($ptypes) ?> distinct data types for parameters, as described below.</p>

<ul>
<? foreach($ptypes as $type => $description) { ?>
	<li><em><?= $type ?></em> - <?= $description ?></li>
<? } ?>
</ul>


<hr/>
<h3>Return Types</h3>
<p>The API supports <?= count($rtypes) ?> formats, which are determined by the server by the filetype extension of the request.</p>

<ul>
<? foreach($rtypes as $type => $description) { ?>
	<li><em><?= $type ?></em> - <?= $description ?></li>
<? } ?>
</ul>

<hr/>
<h3>Status Codes</h3>
<p>The following error codes are used by the site as return values:</p>
<ul>
<? foreach($codes as $code => $description) { ?>
	<li><b><?= $code ?></b> <?= $description ?></li>
<? } ?>
</ul>

<hr/>
<h3>Don't be Retarded</h3>
<p>Like the RSS feeds, requests are limited to a maximum of 200 returned elements.  If you want to download the entire history of the world since the dawn of time for your research project, that's fine but please sleep between requests.</p>

<p>This API does not support the load of TMBO on its own, so you may find some queries are inefficient and take a while to complete.  If you notice calls taking more than two seconds, please <a href="#Errata">send an email or open a ticket</a> with the method/arguments you're requesting, and try to reduce your usage until the problem is addressed.</p>

<hr/>
<h3>More</h3>
<p>The user-agent of the client is used to identify the client.<br />
If you are developing an application, please <a href="#Errata">send an email or open a ticket</a> with your application name and a sample user agent.</p>

<hr/>
<h3>Examples</h3>
<p>Here are some useful examples:</p>
<ul>
	<li>Get a front page of data: <code>https://thismight.be/offensive/api.php/getuploads.plist?type=image&amp;limit=100&amp;token=yourtoken</code></li>
	<li>Get the hall of fame: <code>https://thismight.be/offensive/api.php/getuploads.plist?type=image&amp;limit=100&amp;sort=votes_desc&amp;token=yourtoken</code></li>
</ul>

<p>More examples for each method can be found with its documentation.</p>

<hr/>
<h2>Function Definitions</h2>

<p>To jump to a function, click its link below:</p>
<ul>
<? foreach($methods as $method => $data) { ?>
	<li><a href="#<?= $method ?>"><?= $method ?></a></li>
<? } ?>
</ul>

<? foreach($methods as $method => $data) { ?>
<hr/>
<a name="<?= $method ?>">
<h3><?= $method ?></h3>
<p><?= nl2br(trim($data["desc"])) ?></p>

<? if(count($data["params"])) { ?>
<h4>Arguments:</h4>
<ul>
	<? foreach($data["params"] as $name => $info) { ?>
    <li><b><?= $name ?></b> (<?= $info["type"] ?>) - <?= $info["req"] ?>.
			<? if(strlen($info["values"])) { ?>
				Can be any of <?= $info["values"]?>.
			<? } ?>
			<?= nl2br(trim($info["desc"])) ?></li>
	<? } ?>
</ul>

<? } else { ?>
<p>This method takes no arguments.</p>
<? } ?>

<h4>Returns:</h4>
<? if(!array_key_exists("return", $data)) {
	trigger_error("method $method does not have @return documentation", E_USER_ERROR);
} ?>
<p><?= $data["return"] ?></p>

<h4>Examples:</h4>
<? if(!count($data["examples"])) {
	trigger_error("method $method does not have any examples", E_USER_ERROR);
}
foreach($data["examples"] as $example) { ?>
<p><code><?= "$method.xxx?$example" ?></code>: 
	<? foreach($rtypes as $type => $desc) { ?>
		<a href="/offensive/docs/<?= urlencode("/$method.$type?$example"); ?>"><?= $type ?></a>
	<? } ?>
</p>
<? } ?>

<? if(count($data["see"])) { ?>
<h4>See Also:</h4>

	<? foreach($data["see"] as $see) {
		if (!array_key_exists($see, $methods)) {
			trigger_error("$see is not a valid method, referenced by @see in $method", E_USER_ERROR);
		} ?>
		<p><a href="#<?= $see ?>"><?= $see ?></a> - <?= strpos("\n", $methods[$see]["desc"]) === false ?
		                                               $methods[$see]["desc"] :
		                                               substr($methods[$see]["desc"], 0, strpos("\n", $methods[$see]["desc"])) ?></p>
	<? } ?>
<? } ?>

<? } ?>


<hr />
<a name="Errata">
<h2>Errata</h2>
<p>If the API lacks notable functionality, or operates in an unexpected manner, please report this behaviour (expected and actual, if applicable) to the <a href="mailto:tmbotech@googlegroups.com">tmbotech</a> mailing list, open a <a href="https://github.com/numist/this-might-be-offensive/issues">new issue</a>, or send numist an <a href="mailto:tmbo@numist.net">email</a>.</p>

<hr/>
<h2>Changelog</h2>
<ul>
	<li>August 11, 2008 - Initial draft.</li>
	<li>August 14, 2008 - postuploads should take nsfw and tmbo as arguments.</li>
	<li>August 15, 2008 - searchuploads and searchcomments get more parameters ([limit, type], limit).</li>
	<li>October 18, 2011 - Documentation is now generated/updated by script. Primary input is the documentation in the API code itself.</li>
	
	<li><?= gmdate("F j, Y") ?> - This page was generated.</li>
</ul>
</body>
</html>
