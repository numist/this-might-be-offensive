<?
	require_once( '../mysqlConnectionInfo.php' );	
	$link = openDbConnection();

	if( $_REQUEST['action'] == "shipped" ) {
		$orderid = $_REQUEST['orderid'];
		$sql = "UPDATE merch_orders set status='shipped' WHERE id=$orderid LIMIT 1";
		mysql_query( $sql );
		header( "Location: " . $_SERVER['PHP_SELF'] );		
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">
<html>
<head>
	<title></title>
	<meta name="generator" content="BBEdit 7.0.3">
	
	
	<style type="text/css">
	
		body {
			font-family:verdana;
			font-size:11px;
		}

		td {
			padding:4px;
		}

		.shipped {
			color:#666666;
			background:#cccccc;
		}

		.pending {

		}
		
	</style>
	
</head>


<body bgcolor="#ffffff">


	
	<table border="1">
	
	<?

		if( $_REQUEST['status'] ) {
			$status = $_REQUEST['status'];
		}

		$sql = "SELECT *, merch_orders.id as orderid
				FROM	merch_orders,
						merch_buyers,
						merch_order_items,
						merch_items,
						merch_order_item_options
				WHERE buyer_id=merch_buyers.id
					AND order_id=merch_orders.id
					AND merch_items.id=merch_order_items.item_id
					AND order_item_id=merch_order_items.id";

		if( isset( $status ) ) {
			$sql .= " AND status = '$status' ";
		}

		$sql .=	" GROUP BY order_id";
	
		$result = mysql_query( $sql ) ;
	
		while( $row = mysql_fetch_assoc( $result ) ) {
	?>
			<tr class="<? echo $row['status'] ?>">
				<td><? echo $row['orderid'] ?></td>
				<td><? echo $row['timestamp'] ?></td>
				<td>
					<a href="mailto:<? echo $row['email']?>?subject=[themaxx] Your order"><? echo $row['address_name']?></a><br/>
					<? echo $row['street'] ?><br/>
					<? echo $row['city'] ?>, <? echo $row['state'] ?> <? echo $row['zip'] ?><br/>
				</td>
				<td><? echo $row['description'] ?></td>
				<td><? echo $row['option_value'] ?></td>
				<td><? echo $row['amount'] ?></td>
				<td><? echo $row['status'] ?></td>				
				<td>
					<? if( $row['status'] != "shipped" ) { ?>
						<a href="orders.php?action=shipped&orderid=<?php echo $row['orderid'] ?>">mark shipped</a>
					<? } ?>
				</td>
			</tr>

	<?	
		}
	
	?>

	</table>
	


</body>
</html>
