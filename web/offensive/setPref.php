<?php
	session_start();

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ./logn.php?redirect=" . urlencode( $_SERVER['PHP_SELF'] . "?" . $_SERVER['QUERY_STRING'] ));
		exit;
	}

	require '../admin/mysqlConnectionInfo.php';
	require 'getPrefs.php';

	$link = openDbConnection();

	$prefid = mysql_real_escape_string( $_REQUEST['p'] );
	$valueid = mysql_real_escape_string( $_REQUEST['v'] );

	if( $prefid ) {
	
		$sql = "SELECT user_preferences.id FROM user_preferences WHERE nameid=$prefid AND userid = " . $_SESSION['userid'];	
		
		$result = mysql_query( $sql );
		
		if( mysql_num_rows( $result ) > 0 ) {
			$row = mysql_fetch_array( $result );
			$rowid = $row['id'];
			if( $valueid != "" ) {
				// if we have a value and an existing preference, update it.
				$sql = "UPDATE user_preferences SET valueid=$valueid WHERE id=$rowid";
			}
			else {
				// if we have an existing value but no new value, delete the existing record
				$sql = "DELETE FROM user_preferences WHERE id=$rowid";
			}
		}
		else {
			// if no preference for this pref name and user exists, add it.
			$sql = "INSERT INTO user_preferences (userid, nameid, valueid) VALUES ( " . $_SESSION['userid'] . ", $prefid, $valueid )";
		}
		
		$result = mysql_query( $sql );
		
		$_SESSION['prefs'] = getPreferences( $_SESSION['userid'] );
	
	}
	
	if( is_numeric( $_REQUEST['sq'] ) ) {
		$squelch = mysql_real_escape_string( $_REQUEST['sq'] );
		$sql = "insert into offensive_squelch (userid, squelched) VALUES ( " . $_SESSION['userid'] . ", $squelch )";
		mysql_query( $sql );
		$prefs = getPreferences( $_SESSION['userid'] );
		$_SESSION['prefs'] = $prefs;
	}

	if( is_numeric( $_REQUEST['unsq'] ) ) {
		$squelch = mysql_real_escape_string( $_REQUEST['unsq'] );
		$userid = $_SESSION['userid'];
		$sql = "delete from offensive_squelch where userid=$userid AND squelched=$squelch";
		mysql_query( $sql );
		$prefs = getPreferences( $_SESSION['userid'] );
		$_SESSION['prefs'] = $prefs;
	}


	header( "Location: " . $_SERVER['HTTP_REFERER'] );

?>
