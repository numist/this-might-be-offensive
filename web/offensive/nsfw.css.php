<?php header( "Content-type: text/css" );
session_start();
?>

	<?php
		if( $_SESSION['prefs']['hide nsfw'] == 1 ) {
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


