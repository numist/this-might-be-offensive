<?

	require_once( 'offensive/assets/tabs.inc' );	
	require_once( 'offensive/assets/getPrefs.inc' );	

	function start() {
		if(!array_key_exists("userid", $_REQUEST) || 
		   !is_numeric($_REQUEST['userid'])) {
			header("Location: ./");
			exit;
		}
	}
	
	function isAdmin() {
		return $_SESSION['status'] == "admin";
	}
	
	function body() {
		global $link;

		$usrid = $_REQUEST['userid'];

		// Include, and check we've got a connection to the database.
		require_once( '../admin/mysqlConnectionInfo.inc' );
		if(!isset($link) || !$link) $link = openDbConnection();
				
		$sql = "SELECT username
					FROM users WHERE userid={$usrid}";
		$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
		list($username) = mysql_fetch_array( $result );

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
			$result = mysql_query( $sql ) or trigger_error(mysql_error(), E_USER_ERROR);
	
		
			while( list($name, $status, $created, $userid, $referrer) = mysql_fetch_array($result)) {
				?><li><a class="username <?=$status?>" href="./?c=user&userid=<?=$userid?>"><?= $name ?></a> (<?= $status ?>)</li>
<?
			}
	?>											
					</ol>
					</div>
		



<?
}
