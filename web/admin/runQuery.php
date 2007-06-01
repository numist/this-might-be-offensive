<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 6.0.2">
	
	
</head>


<body bgcolor="#ffffff">

<form action="runQuery.php" method="post">
	<textarea name="query" rows="20" cols="80"><?php echo $query?></textarea><br>
	<input type="submit" value="go!">
</form>

<table width="100%" border="1" cellpadding="4" cellspacing="0">

<?php if( $query ) {
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();

	$result = mysql_query($query) or die("Query failed");

	// get the results of the query as an associative array, indexed by column name
	while( $row = mysql_fetch_array( $result, MYSQL_NUM ) ) {
		echo "<tr>\n";
		foreach ($row as $col_value) {
            echo "\t\t<td> $col_value</td>\n";
        }
        echo "</tr>\n";
	}

} 
?>

</table>


</body>
</html>
