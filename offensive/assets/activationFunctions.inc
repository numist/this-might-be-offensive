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