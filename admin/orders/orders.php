<?php
	require_once( '../mysqlConnectionInfo.php' );	
	if(!isset($link) || !$link) $link = openDbConnection();

	if( $_REQUEST['action'] == "shipped" ) {
		$orderid = $_REQUEST['orderid'];
		$sql = "UPDATE merch_orders set status='shipped' WHERE id=$orderid LIMIT 1";
		mysqli_query($link, $sql );
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
	
	<?php

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
	
		$result = mysqli_query($link, $sql ) ;
	
		while( $row = mysqli_fetch_assoc( $result ) ) {
	?>
			<tr class="<?php echo $row['status'] ?>">
				<td><?php echo $row['orderid'] ?></td>
				<td><?php echo $row['timestamp'] ?></td>
				<td>
					<a href="mailto:<?php echo $row['email']?>?subject=[themaxx] Your order"><?php echo $row['address_name']?></a><br/>
					<?php echo $row['street'] ?><br/>
					<?php echo $row['city'] ?>, <?php echo $row['state'] ?> <?php echo $row['zip'] ?><br/>
				</td>
				<td><?php echo $row['description'] ?></td>
				<td><?php echo $row['option_value'] ?></td>
				<td><?php echo $row['amount'] ?></td>
				<td><?php echo $row['status'] ?></td>				
				<td>
					<?php if( $row['status'] != "shipped" ) { ?>
						<a href="orders.php?action=shipped&orderid=<?php echo $row['orderid'] ?>">mark shipped</a>
					<?php } ?>
				</td>
			</tr>

	<?php 	
		}
	
	?>

	</table>
	


</body>
</html>
