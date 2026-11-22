<?php
	// XXX: this needs to use core
	//
	// $id distinguishes render sites: this is drawn both in the sidebar and inside
	// the tabs popover, and two elements sharing id="unread" is invalid HTML that
	// breaks getElementById and #unread styling for whichever comes second.
	function unread($id = "unread") {
		if(!me()) return;
		$uid = me()->id();

		// Memoized because both render sites call this on the same request and the
		// query is identical; without this the page pays for it twice.
		static $comments = null;
		if($comments === null) $comments = core_unreadcomments(array());

		$isEmpty = count($comments) == 0;

		?>

		<div id="<?= htmlEscape($id) ?>" class="contentbox <?= ($isEmpty ? 'empty' : '')?>">
			<div class="heading">unread comments:</div>
			<div id="<?= htmlEscape($id) ?>-container" class="bluebox">

				<?php 
					if($isEmpty) {
						?>all caught up.<?php
					}
				?>

				<?php foreach ($comments as $comment) {
					$upload = $comment->upload();
					if($upload->squelched()) continue;

					$css = isset($css) && $css == "evenfile" ? "oddfile" : "evenfile";  ?>
					<div class="clipper"><a id="<?= htmlEscape($id) ?><?= $comment->upload()->id()?>" class="<?= $css ?>" href="<?= Link::comment($comment) ?>"><?= $upload->htmlFilename() ?></a></div>
				<?php } ?>
			</div>
		</div>
	<?php }
?>