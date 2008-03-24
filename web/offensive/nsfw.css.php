<?php header( "Content-type: text/css" );
session_start();
?>
<?php
	if( array_key_exists("prefs", $_SESSION) &&
	    is_array($_SESSION['prefs']) &&
	    array_key_exists("hide nsfw", $_SESSION["prefs"]) &&
	    $_SESSION['prefs']['hide nsfw'] == 1 ) {
	?>		

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

	<?php
		}
	?>