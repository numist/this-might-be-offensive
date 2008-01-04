<?php
	set_include_path("..");
	require_once('offensive/assets/header.inc');

	if( ! is_numeric( $_SESSION['userid'] ) ) {
		header( "Location: ./" );
		exit;
	}

	require_once( 'admin/mysqlConnectionInfo.inc' ); 
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/getPrefs.inc');
	require_once('offensive/assets/functions.inc');

	$prefid = sqlEscape( array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : "");
	$valueid = sqlEscape( array_key_exists("v", $_REQUEST) ?$_REQUEST['v'] : "");

	if( strlen($prefid) > 0 ) {
	
		$sql = "SELECT user_preferences.id FROM user_preferences WHERE nameid=$prefid AND userid = " . $_SESSION['userid'];	
		
		$result = tmbo_query( $sql );
		
		if( mysql_num_rows( $result ) > 0 ) {
			$row = mysql_fetch_array( $result );
			$rowid = $row['id'];
			if( $valueid != "" ) {
				// if we have a value and an existing preference, update it.
				$sql = "UPDATE user_preferences SET valueid=$valueid WHERE id=$rowid";
			} else {
				// if we have an existing value but no new value, delete the existing record
				$sql = "DELETE FROM user_preferences WHERE id=$rowid";
			}
		} else {
			// if no preference for this pref name and user exists, add it.
			$sql = "INSERT INTO user_preferences (userid, nameid, valueid) VALUES ( " . $_SESSION['userid'] . ", $prefid, $valueid )";
		}
		
		$result = tmbo_query( $sql );
		
		$_SESSION['prefs'] = getPreferences( $_SESSION['userid'] );
	
	}
	
	if( array_key_exists("sq", $_REQUEST) && is_numeric( $_REQUEST['sq'] ) ) {
		$squelch = sqlEscape( $_REQUEST['sq'] );
		$sql = "insert into offensive_squelch (userid, squelched) VALUES ( " . $_SESSION['userid'] . ", $squelch )";
		tmbo_query( $sql );
		$prefs = getPreferences( $_SESSION['userid'] );
		$_SESSION['prefs'] = $prefs;
	}

	if( array_key_exists("unsq", $_REQUEST) && is_numeric( $_REQUEST['unsq'] ) ) {
		$squelch = sqlEscape( $_REQUEST['unsq'] );
		$userid = $_SESSION['userid'];
		$sql = "delete from offensive_squelch where userid=$userid AND squelched=$squelch";
		tmbo_query( $sql );
		$prefs = getPreferences( $_SESSION['userid'] );
		$_SESSION['prefs'] = $prefs;
	}


	if(array_key_exists("HTTP_REFERER", $_SERVER)) {
		header( "Location: " . $_SERVER['HTTP_REFERER'] );
	} else {
		echo "<html><head><script type=\"text/javascript\">history.go(-1);</script></head><body /></html>";
	}

?>
