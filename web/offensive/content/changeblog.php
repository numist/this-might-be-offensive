<?
	function body() {
?>
<div class="heading">beat to fit. paint to match.</div>
<div class="bluebox">




<?
//	include( './changeblog/index.html' );
	require_once( '../admin/mysqlConnectionInfo.php' );
	
	$link = openDbConnection();

	$sql = "SELECT  entry_id, entry_text, entry_created_on
				FROM  mt_entry 
				WHERE entry_blog_id = 3
				ORDER  BY entry_created_on DESC";
				
	$result = mysql_query( $sql );

	while( $row = mysql_fetch_assoc( $result ) ) {
		$css = ($css == "even_row") ? "odd_row" : "even_row";
		?>
		<div class="entry" style="<?php echo nextStyle()?>">
			<?= $row['entry_text'] ?><br/>
			<div class="timestamp"><a href="#<?= $row['entry_id'] ?>"><?= $row['entry_created_on'] ?></a></div>
			<br/>
		</div>
		<?
	}
?>



</div>
<?
	}
	
	function nextStyle() {
		global $style;
		$style = $style == "background:#bbbbee;" ? "" : "background:#bbbbee;";
		return $style;
	}

?>
