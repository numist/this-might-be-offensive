<?

	require_once( 'tabs.php' );	
	require_once( 'getPrefs.php' );	

	function start() {
	
		$usrid = $_REQUEST['userid'];
		if( ! is_numeric( $usrid ) ) {
			header( "Location: ./" );
		}

	}
	
	function isAdmin() {
		$uid = $_SESSION['userid'];
		return ($uid == 151 || $uid == 200 || $uid == 87 );
	}
	
	function body() {

		$usrid = $_REQUEST['userid'];

		$link = openDbConnection();

				
		$sql = "SELECT username
					FROM users WHERE userid={$usrid}";
		list($username) = mysql_fetch_array( mysql_query( $sql ) );		

?>

					<div class="heading">
						<span style="color:#666699">
							<a href="./?c=user&userid=<?=$usrid?>"><?= $username?></a> has a posse.
						</span>
					</div>
					<? tabs(); ?>
					<div class="bluebox">
						<style type="text/css">
							.normal {
							
							}
							
							.locked {
								color:#999999;
							}
						</style>
						<div class="piletitle">
							<a href="./?c=user&userid=<?=$usrid?>"><?= $username?></a> has a posse.
						</div>
						<ol>
	<?
				
			$sql = "SELECT username, account_status, created, userid, 
						(select username from users where userid={$usrid}) AS referrer
						FROM users WHERE referred_by={$usrid}
						ORDER BY username";
			$result = mysql_query( $sql );
	
		
			while( list($name, $status, $created, $userid, $referrer) = mysql_fetch_array($result)) {
				?><li><a class="username <?=$status?>" href="./?c=user&userid=<?=$userid?>"><?= $name ?></a> (<?= $status ?>)</li>
<?
			}
	?>											
					</ol>
					</div>
		



<?
}
