<?php

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
						<span style="color:var(--link-user)">
							<?= $them->htmlUsername() ?> has a posse.
						</span>
					</div>
					<?php tabs(); ?>
					<div class="bluebox">
						<h3><?= $them->htmlUsername() ?> has a posse.</h3>
						<ol>
<?php
			
			$posse = $them->posse();
			foreach($posse as $accomplice) {
				?><li><?= $accomplice->htmlUsername() ?> (<?= $accomplice->status() ?>)</li>
<?php
			}
?>											
					</ol>
					</div>
					<div class="blackbar"></div>
				</div>
<?php
}
?>