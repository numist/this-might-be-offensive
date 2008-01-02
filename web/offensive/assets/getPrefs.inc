<?php
function getPreferences( $userid ) {
	// Include, and check we've got a connection to the database.
	require_once( '../admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
		
	$sql = "SELECT names.value AS prefname, val.value AS prefvalue FROM user_preferences, preference_names_values names, preference_names_values val WHERE userid=$userid AND nameid=names.id AND val.id=valueid";

	$result = tmbo_query($sql);

	// get the results of the query as an associative array, indexed by column name
	while( $row = mysql_fetch_array( $result, MYSQL_ASSOC ) ) {
		$prefs[ $row['prefname'] ] = $row['prefvalue'];
	}


	$squelched = array();

	$sql = "SELECT squelched FROM offensive_squelch WHERE userid=$userid";
	$result = tmbo_query($sql);

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
