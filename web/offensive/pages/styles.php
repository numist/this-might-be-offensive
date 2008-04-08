<?php 
set_include_path("../..");
require_once( 'offensive/assets/header.inc' );

header( "Content-type: text/css" );
?>

#comments_link {
	padding-left:24px;
}

#content {
	margin:8px;
}

body {
	padding:0px;
	margin:0px;
	font-family:verdana;
	font-size:11px;

}

#votelinks.off, #votelinks.off a {
	color:#ccc;
	text-decoration:none;
}

.nsfw {

	<?php
		if( array_key_exists("prefs", $_SESSION) &&
		    is_array($_SESSION['prefs']) &&
		    array_key_exists("hide nsfw", $_SESSION["prefs"]) &&
		    $_SESSION['prefs']['hide nsfw'] == 1 ) {
	?>		display:none; <?php
		}
	?>
}
