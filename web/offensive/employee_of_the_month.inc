employee of the month: 
<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	$sql = 'SELECT count( vote ) AS thecount, fileid, filename, offensive_uploads.timestamp, username, users.userid FROM offensive_comments, offensive_uploads, users WHERE vote = \'this is good\' AND fileid = offensive_uploads.id AND offensive_uploads.userid = users.userid AND offensive_uploads.timestamp > DATE_SUB( now( ) , INTERVAL 1 MONTH ) GROUP BY offensive_uploads.userid ORDER BY thecount DESC LIMIT 1';
	$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	$row = mysql_fetch_assoc( $result );
	
	echo "<a class=\"orange\" href=\"user.php?userid=" . $row[ 'userid' ] . "\">" . $row[ 'username' ] . "</a> (+" . $row['thecount'] . ")";
	
?>
