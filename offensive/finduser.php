<?php
	set_include_path("..");
	require_once( 'offensive/assets/header.inc' );
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once('offensive/assets/functions.inc');
	require_once('offensive/assets/validationFunctions.inc');
	mustLogIn();

	/* isValidUsername is anchored and its character set holds no quote, backslash or
	 * space, so a name that passes cannot break out of the literal, and no stored
	 * name can carry a trailing space -- which is the only way "=" and LIKE differ,
	 * so "=" is an exact match here.  That also disposes of "_", which LIKE reads as
	 * a single-character wildcard: the regex allows it in a username, so LIKE matched
	 * names other than the one asked for, and the one-match redirect turned that into
	 * a search oracle.
	 *
	 * sqlEscape stays anyway.  That regex is already what keeps registr.php's
	 * unescaped inserts and htmlUsername()'s output sinks safe; widening it one day
	 * for a display name should not quietly reach into this query too.
	 */
	$userid = null;
	if( isValidUsername( $_REQUEST['finduser'] ) ) {
		$sql = "SELECT userid FROM users WHERE username = '" . sqlEscape( $_REQUEST['finduser'] ) . "'";
		$result = tmbo_query($sql);

		if( mysql_num_rows( $result ) == 1 ) {
			$row = mysql_fetch_array( $result );
			$userid = $row['userid'];
		}
	}

	if( is_null( $userid ) ) {
		header("Location: ".$_SERVER['HTTP_REFERER']);
	}
	else {
		header("Location: ".Link::user($userid));
	}

?>
