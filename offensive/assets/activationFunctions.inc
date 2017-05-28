<?

	$id_offset = 3737;
	$pad_length = 8;
	$hash_param_key = "z";
	$salt = ":woot";

	// generates a string used in the link
	// to identify the account to be activated
	// without being completely obvious.
	function tmbohash( $id, $input ) {
		global $id_offset, $pad_length;
		$padded_id = str_pad( $id + $id_offset, $pad_length, "0", STR_PAD_LEFT );
		$crypted_input = sha1( $input.$id );
		return base64_encode( $padded_id . $crypted_input );
	}
	
	// returns the descrambled id
	// from a string created by hash()
	function id_from_hash( $input ) {
		global $id_offset, $pad_length;
		$temp = base64_decode( $input );
		$id = substr( $temp, 0, $pad_length ) - $id_offset;
		return $id;
	}

	function isOpenRegistration() {
		// users table doens't have an index on created, so grab the highest userid in a subquery since it must be the most recently registered
		$sql = "SELECT COUNT(*) FROM (SELECT * FROM users ORDER BY userid DESC LIMIT 1) AS t WHERE created > DATE_SUB(NOW(), INTERVAL 1 DAY)";
		$result = tmbo_query( $sql );
		$recentRegistrations = mysql_fetch_array( $result )[0];

		if ($recentRegistrations != 0) {
			return false;
		}

		$sql = "SELECT COUNT(*) FROM users WHERE last_login_ip = \"".sqlEscape($_SERVER['REMOTE_ADDR'])."\" AND account_status = \"locked\"";
		$result = tmbo_query( $sql );
		$bannedIPs = mysql_fetch_array( $result )[0];

		return $bannedIPs == 0;
	}

	function activationMessageFor( $id, $email ) {
		global $hash_param_key, $salt;

		$hash = tmbohash( $id, $email . $salt );
		$message = "  Thanks for registering at [ this might be offensive ].
  Please visit this link to activate your account:
  https://thismight.be/offensive/activate.php?$hash_param_key=$hash
  
  Do not respond to this email. If you need to contact us,
  please visit http://thismight.be/contact/

";

		return $message;

	}

?>