<?
	// Include, and check we've got a connection to the database.
	include_once( '../admin/mysqlConnectionInfo.php' ); $link = openDbConnection();


	function body() {

		$find = $_REQUEST['findfile'];
	
		$sql = "SELECT id, filename, offensive_uploads.timestamp, users.userid, username, offensive_uploads.type
					FROM offensive_uploads, users
					WHERE filename LIKE '%$find%'
					AND offensive_uploads.userid = users.userid
					ORDER BY offensive_uploads.timestamp DESC
					LIMIT 100;
				";

		$link = openDbConnection();
		$result = mysql_query( $sql );
		
		
?>

	<div class="heading">search results</div>
	<div class="bluebox">
	
	
	
<table width="100%">


<?
		if( mysql_num_rows( $result ) == 0 ) {
			echo "<div class=\"piletitle\">No dice, but thanks for playing.</div>";
		}

		while( $row = mysql_fetch_assoc( $result ) ) {
			
			$css = ($css == "evenfile") ? "oddfile" : "evenfile";
			$type = $row['type'];
			$expired = $type == 'image' && strtotime( $row['timestamp'] ) < time() - 60 * 60 * 24 * 3 ? "(expired)" : ""
?>
	
			<tr class="<? echo $css ?>">
				<td><? echo date( "Y-m-d", strtotime( $row['timestamp'] ) ) ?></td>
				<? if( $type == 'image' ) { ?>
					<td><a class="<?echo $css ?>" href="./pages/pic.php?id=<? echo $row['id'] ?>"><? echo $row['filename'] ?></a> <? echo  $expired ?></td>
				<? } ?>
				<? if( $type == 'topic' ) { ?>
					<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>"><?= $type ?>: <? echo $row['filename'] ?></a></td>
				<? } ?>
				<td><a class="<?echo $css ?>" href="./?c=comments&fileid=<? echo $row['id'] ?>">comments</td>
			</tr>
	
<?	
		}
?>


</table>



	</div>

<?
		
	}

?>
