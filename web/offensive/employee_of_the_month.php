employee of the month: 
<?php

	require_once '../admin/mysqlConnectionInfo.php';
	$link = openDbConnection();
	$sql = 'SELECT count( vote ) AS thecount, fileid, filename, offensive_uploads.timestamp, username, users.userid FROM offensive_comments, offensive_uploads, users WHERE vote = \'this is good\' AND fileid = offensive_uploads.id AND offensive_uploads.userid = users.userid AND users.username != \'Fipi Lele\' AND offensive_uploads.timestamp > DATE_SUB( now( ) , INTERVAL 1 MONTH ) GROUP BY offensive_uploads.userid ORDER BY thecount DESC LIMIT 1';
	$result = mysql_query( $sql );
	$row = mysql_fetch_assoc( $result );
	
	echo "<a class=\"orange\" href=\"user.php?userid=" . $row[ 'userid' ] . "\">" . $row[ 'username' ] . "</a> (" . $row['thecount'] . " good votes)";
	
?>
