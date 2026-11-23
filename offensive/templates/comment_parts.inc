<?php
// Helpers for comment.inc. Kept in their own file because comment.inc is
// require()d once per comment; declaring functions there redeclares them
// (fatal) on the second comment. This file is loaded via require_once.

function commentMeta ($comment, &$numgood, &$numbad) {
	$ts = strtotime($comment->timestamp());
?>
	<header class="comment_meta">
		<?= $comment->commenter()->htmlUsername() ?>
		<?php commentVotes($comment, $numgood, $numbad); ?>
		<a href="<?= Link::comment($comment) ?>"><time class="timestamp"<?php if($ts) { ?> datetime="<?= date('c', $ts) ?>"<?php } ?>><?= $comment->timestamp() ?></time></a>
	</header>
<?php
}

// the vote and the tmbo/repost flags, shown beside the username in
// commentMeta(). $numgood/$numbad are running tallies shared across the
// comment loop, so they're taken by reference.
function commentVotes ($comment, &$numgood, &$numbad) {
	$thevote = $comment->vote();
	if($thevote) {
		$numgood += ($thevote == 'this is good') ? 1 : 0;
		$numbad += ($thevote == 'this is bad') ? 1 : 0;
	}
	$tmbo = $comment->tmbo() == 1;
	$tiar = $comment->tiar() == 1;

	if($thevote || $tmbo || $tiar) {
?>
		<span class="comment_votes">
			<?php if($thevote) { ?><span title="+<?= $numgood ?> -<?= $numbad ?>" class="vote <?= strpos($thevote, 'good') > 0 ? 'good' : 'bad' ?>"><span>[ <?= $thevote ?> ]</span></span><?php } ?>
			<?php if($tmbo) { ?><span class="vote tmbo"><span>[ this might be offensive ]</span></span><?php } ?>
			<?php if($tiar) { ?><span class="vote tiar"><span>[ this is a repost ]</span></span><?php } ?>
		</span>
<?php
	}
}
