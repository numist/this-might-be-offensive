<?php
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

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
	
	// assign posted variables to local variables
	$item_name = mysqli_real_escape_string($link, $_POST['item_name'] );
	$option_name = mysqli_real_escape_string($link, $_POST['option_name1'] );
	$size = mysqli_real_escape_string($link, $_POST['option_selection1'] );	
	$item_number = mysqli_real_escape_string($link, $_POST['item_number'] );
	$payment_status = mysqli_real_escape_string($link, $_POST['payment_status'] );
	$payment_amount = mysqli_real_escape_string($link, $_POST['mc_gross'] );
	$payment_currency = mysqli_real_escape_string($link, $_POST['mc_currency'] );
	$txn_id = mysqli_real_escape_string($link, $_POST['txn_id'] );
	$receiver_email = mysqli_real_escape_string($link, $_POST['receiver_email'] );
	$payer_email = mysqli_real_escape_string($link, $_POST['payer_email'] );
	
	$first_name = mysqli_real_escape_string($link, $_POST['first_name'] );	
	$last_name = mysqli_real_escape_string($link, $_POST['last_name'] );	
	$street = mysqli_real_escape_string($link, $_POST['address_street'] );	
	$city = mysqli_real_escape_string($link, $_POST['address_city'] );	
	$state = mysqli_real_escape_string($link, $_POST['address_state'] );	
	$zip = mysqli_real_escape_string($link, $_POST['address_zip'] );
	$country = mysqli_real_escape_string($link, $_POST['address_country'] );	
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
				
				$result = mysqli_query($link, $sql );

				$err = mysqli_error($link);

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}

				$buyer_id = mysqli_insert_id($link);

				$sql = "INSERT INTO merch_orders ( transaction_id, amount, buyer_id, payment_status )
						VALUES ( '$txn_id', $amount, $buyer_id, '$payment_status' )
				";

				$result = mysqli_query($link, $sql );

				$err = mysqli_error($link);

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}

				$order_id = mysqli_insert_id($link);
				
				$sql = "INSERT INTO merch_order_items ( order_id, item_id )
						VALUES ( $order_id, $item_number )";

				$result = mysqli_query($link, $sql );

				$err = mysqli_error($link);

				if( $err != "" ) {
					reportError( $err, $sql );
					die;
				}
				
				$order_item_id = mysqli_insert_id($link);

				$sql = "INSERT INTO merch_order_item_options ( order_item_id, option_name, option_value )
						VALUES ( $order_item_id, '$option_name', '$size' )";

				$result = mysqli_query($link, $sql );

				$err = mysqli_error($link);

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
