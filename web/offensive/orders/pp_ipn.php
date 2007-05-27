<?
	require_once( '../../admin/mysqlConnectionInfo.php' );

	// read the post from PayPal system and add 'cmd'
	$req = 'cmd=_notify-validate';
	
	foreach ($_POST as $key => $value) {
		$value = urlencode(stripslashes($value));
		$req .= "&$key=$value";
	}
	
	// post back to PayPal system to validate
	$header .= "POST /cgi-bin/webscr HTTP/1.0\r\n";
	$header .= "Content-Type: application/x-www-form-urlencoded\r\n";
	$header .= "Content-Length: " . strlen($req) . "\r\n\r\n";
	$fp = fsockopen ('www.paypal.com', 80, $errno, $errstr, 30);


	$link = openDbConnection();
	
	// assign posted variables to local variables
	$item_name = mysql_real_escape_string( $_POST['item_name'] );
	$option_name = mysql_real_escape_string( $_POST['option_name1'] );
	$size = mysql_real_escape_string( $_POST['option_selection1'] );	
	$item_number = mysql_real_escape_string( $_POST['item_number'] );
	$payment_status = mysql_real_escape_string( $_POST['payment_status'] );
	$payment_amount = mysql_real_escape_string( $_POST['mc_gross'] );
	$payment_currency = mysql_real_escape_string( $_POST['mc_currency'] );
	$txn_id = mysql_real_escape_string( $_POST['txn_id'] );
	$receiver_email = mysql_real_escape_string( $_POST['receiver_email'] );
	$payer_email = mysql_real_escape_string( $_POST['payer_email'] );
	
	$first_name = mysql_real_escape_string( $_POST['first_name'] );	
	$last_name = mysql_real_escape_string( $_POST['last_name'] );	
	$street = mysql_real_escape_string( $_POST['address_street'] );	
	$city = mysql_real_escape_string( $_POST['address_city'] );	
	$state = mysql_real_escape_string( $_POST['address_state'] );	
	$zip = mysql_real_escape_string( $_POST['address_zip'] );
	$country = mysql_real_escape_string( $_POST['address_country'] );	
	$amount = $_POST['mc_gross'];	

	if (!$fp) {
		// HTTP ERROR
	} else {
		fputs ($fp, $header . $req);

		$file = fopen( "ipn.txt", "a+" );

		fwrite( $file, "!\n" );

		while (!feof($fp)) {
			$res = fgets ($fp, 1024);
				
			fwrite( $file, $res . "\n" );
			
			if (strcmp ($res, "VERIFIED") == 0) {
				// check the payment_status is Completed
				// check that txn_id has not been previously processed
				// check that receiver_email is your Primary PayPal email
				// check that payment_amount/payment_currency are correct
				// process payment

				$sql = "INSERT INTO merch_buyers ( first_name, last_name, address_name, street, city, state, zip, country, email, notify_version )
VALUES ( '$first_name', '$last_name', '$address_name', '$street', '$city', '$state', '$zip', '$country', '$payer_email', '$notify_version' )
				";
				
				$result = mysql_query( $sql );

				$err = mysql_error();

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}

				$buyer_id = mysql_insert_id();

				$sql = "INSERT INTO merch_orders ( transaction_id, amount, buyer_id, payment_status )
						VALUES ( '$txn_id', $amount, $buyer_id, '$payment_status' )
				";

				$result = mysql_query( $sql );

				$err = mysql_error();

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}

				$order_id = mysql_insert_id();
				
				$sql = "INSERT INTO merch_order_items ( order_id, item_id )
						VALUES ( $order_id, $item_number )";

				$result = mysql_query( $sql );

				$err = mysql_error();

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}
				
				$order_item_id = mysql_insert_id();

				$sql = "INSERT INTO merch_order_item_options ( order_item_id, option_name, option_value )
						VALUES ( $order_item_id, '$option_name', '$size' )";

				$result = mysql_query( $sql );

				$err = mysql_error();

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}				

				
				
			}
			else if (strcmp ($res, "INVALID") == 0) {
				fwrite( $file, requestDetail() );
			}
		}
		fclose ($fp);
		
		fclose( $file );
	}


	function reportError( $err, $sql ) {
		mail( "ray@mysocalled.com", "ERROR PROCESSING ORDER", "$err				
$sql

" . requestDetail(), "From: pp_ipn.php@themaxx.com" );
	}



	function requestDetail() {
		ob_start();
		var_dump( $_SERVER );
		var_dump( $_REQUEST );		
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}

?>
