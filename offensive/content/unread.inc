<?php
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once( 'offensive/assets/functions.inc' );
	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	function start() {
		mustLogIn();
	}

    function body() {
?>
<div class="contentbox">
  <div class="blackbar"></div>
  <div class="heading">holy shit. look what happened while you were gone.</div>
  <?php tabs(); ?>
  <div class="bluebox"><?php unread_list() ?></div>
</div>
<?php } ?>

<?php
    function unread_list() {
        if(!me()) return;
		$uid = me()->id();
		
		$comments = core_unreadcomments(array());

        if (count($comments) == 0) {
            ?>All caught up. Now what?<?php
        }
        else {
            ?><ul class="unreadlist"><?php
            foreach ($comments as $comment) {
                $upload = $comment->upload();
                if($upload->squelched()) continue;
                require("offensive/templates/listitem_unread.inc");
           }
           ?></ul><?php
        }
    }
?>
