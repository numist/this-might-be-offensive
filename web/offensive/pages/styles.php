<?php header( "Content-type: text/css" );
session_start();
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

.nsfw {

	<?php
		if( $_SESSION['prefs']['hide nsfw'] == 1 ) {
	?>		display:none; <?php
		}
	?>
}
