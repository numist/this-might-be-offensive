<?php
$output = shell_exec('ls -lart');
echo "<pre>$output</pre>";

shell_exec( "convert -resize 100x100 \"uploads/2007/02/14/image/188696.jpg\" \"uploads/2007/02/14/image/th188696.jpg\"" );


?> 