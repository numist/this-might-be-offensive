<?

	require_once( 'offensive/assets/tabs.inc' );	
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	function start() {
		mustLogIn();
	}
	
	function body() {
		$usrid = $_REQUEST['userid'];
		$them = new User($usrid);

?>
<div class="contentbox">
	<div class="blackbar"></div>
					<div class="heading">
						<span style="color:#666699">
							<?= $them->htmlUsername() ?> has a posse.
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
							<?= $them->htmlUsername() ?> has a posse.
						</div>
						<ol>
<?
			
			$posse = $them->posse();
			foreach($posse as $accomplice) {
				?><li><?= $accomplice->htmlUsername() ?> (<?= $accomplice->status() ?>)</li>
<?
			}
?>											
					</ol>
					</div>
					<div class="blackbar"></div>
				</div>
<?
}
?>