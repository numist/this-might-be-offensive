<?
	set_include_path("..");
	require_once( 'offensive/assets/header.inc' );

	require_once( "offensive/activationFunctions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	
	$message = "There was a problem with your request.";	
	
	$id = id_from_hash( $_REQUEST[ $hash_param_key ] );
	
	$sql = "SELECT username,email,account_status from users where userid=$id";
	
	$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	
	if( mysql_num_rows( $result ) == 1 ) {
		
		$row = mysql_fetch_assoc( $result );
		
		$email = $row['email'];
		$username = $row['username'];		
		
		$rehash = tmbohash( $id + 0, $email . $salt );
		
		if( $rehash == $_REQUEST[ $hash_param_key ] ) {
			$sql = "update users set account_status='normal' where userid=$id AND account_status='awaiting activation' limit 1";
			mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
			if( mysql_affected_rows() == 1 ) {
				$message = "Your account is now active. <a href=\"./\">Click here</a> to return to the list.";
			}
		}		
	}
	
?>


<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 7.0.3">

	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>

	<style type="text/css">
		.vote, label {
			font-weight:bold;
			font-size:9px;
		}
		
		#content {
			margin-left:auto;
			margin-right:auto;
			text-align:left;
			line-height: 15px;
			width:771px;
		}
		
		#rightcol {
			width:584px;
			float:left;
			margin-right: auto;
			margin-left: 0px;
		}
		
	</style>

</head>


<body>

 <?php include( $DOCUMENT_ROOT . "/includes/headerbuttons.txt" );?>


<div id="content">

	<div id="titleimg"><a href="./"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="this_might_be_offensive" width="285" height="37" border="0"></a></div>


	<div id="leftcol">

		<div class="contentbox">
			<div class="blackbar"></div>
				<div class="heading">bandwidth isn't free:</div>
				<div class="bluebox">
					<p>help keep this thing running. please make a small donation to help pay for bandwidth.<p>
					<div style="text-align:center">
										
						<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
							<input type="hidden" name="cmd" value="_xclick">
		
							<input type="hidden" name="business" value="bandwidth@themaxx.com">
							<input type="hidden" name="item_name" value="[ this might be offensive ] bandwidth">
							<input type="hidden" name="no_shipping" value="1">
							<input type="hidden" name="no_note" value="1">
							<input type="hidden" name="currency_code" value="USD">
							<input type="hidden" name="tax" value="0">
							<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but04.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
						</form>
						
					</div>
		
				</div>
			<div class="blackbar"></div>
		</div>
	
	
		<div class="contentbox">
			<div class="blackbar"></div>
				<div class="heading">web hosting provided by:</div>
				<div class="bluebox" style="text-align:center">

					<a href="http://www.dreamhost.com/rewards.cgi"><img src="/graphics/dreamhost.gif" alt="dreamhost" width="88" height="33" hspace="0" vspace="0"></a>
				</div>
			<div class="blackbar"></div>
		</div>
	
	</div>
	<div id="rightcol">


	<div class="contentbox">
		<div class="blackbar"></div>
		<div class="heading">account activation</div>
		<div class="bluebox" style="text-align:left">	
			<div class="entry">
			
				<? echo $message ?>
			
			</div>	
		</div>
	</div>

</div>

</body>
</html>
