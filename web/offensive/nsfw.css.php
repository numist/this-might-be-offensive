<?php
session_start();
set_include_path("..");
require("offensive/assets/header.inc");
require("offensive/assets/classes.inc");

header( "Content-type: text/css" );

if(loggedin()) {
	$me = new User($_SESSION["userid"]);
	if($me->getPref("hide_nsfw") == 1) { ?>		

		.nsfw {
			background-image:url( graphics/th-nsfw.gif );
			background-repeat:no-repeat;
			background-position:center;
			width:116px;
			height:100px;
			display:block;
			vertical-align:center;
		}
		
		.nsfw img {
			visibility:hidden;
		}
    	
		.nsfw:hover img {
			visibility:inherit;
		}
		<? 
	}
}
?>