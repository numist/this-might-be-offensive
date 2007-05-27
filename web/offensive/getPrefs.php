<?php
function getPreferences( $userid ) {

	// values defined in mysqlConnectionInfo.php
	global $db_url, $db_user, $db_pw;


	$link = @mysql_connect( $db_url, $db_user, $db_pw )
			or die( "<br><br><br>Unable to connect to database." );

	mysql_select_db("themaxx")
		or die( "<br><br>Could not select database" );
		
	$sql = "SELECT names.value AS prefname, val.value AS prefvalue FROM user_preferences, preference_names_values names, preference_names_values val WHERE userid=$userid AND nameid=names.id AND val.id=valueid";

	$result = mysql_query($sql) or die("Query failed: " . mysql_error() );

	// get the results of the query as an associative array, indexed by column name
	while( $row = mysql_fetch_array( $result, MYSQL_ASSOC ) ) {
		$prefs[ $row['prefname'] ] = $row['prefvalue'];
	}


	$squelched = array();

	$sql = "SELECT squelched FROM offensive_squelch WHERE userid=$userid";
	$result = mysql_query($sql) or die("Query failed: " . mysql_error() );

	// get the results of the query as an associative array, indexed by column name
	while( $row = mysql_fetch_array( $result ) ) {
		array_push( $squelched, $row['squelched'] );
	}

	/* Closing connection */
	mysql_close($link);

	$prefs[ 'squelched' ] = implode( ",", $squelched );
	
	return $prefs;		

}


function isSquelched( $squelchedId ) {
	return in_array( $squelchedId, explode( ",", $_SESSION['prefs']['squelched'] ) );
}

?>