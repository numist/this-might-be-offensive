<?php

// sanity
if(!defined("TMBO") || !isset($comment)) {
	if(!defined("TMBO")) {
		set_include_path("../..");
		define("TMBO", true);
	}
	require("offensive/500.php");
}

$commenter = $comment->commenter();
global $numgood, $numbad;

$ts = strtotime($comment->timestamp());

?>
<article id="<?= $comment->id() ?>" class="entry u<?php echo $commenter->id();
	if($comment->vote() == "this is good") echo " tig";
	if($comment->vote() == "this is bad") echo " tib";
	if($comment->tiar()) echo " tiar";
	if($comment->tmbo()) echo " tmbo";
	if(strlen($comment->text())) echo " comment"; ?>"
	<?php if(isset($css) && $css !== '') { ?>style="<?= $css ?>"<?php } ?>>

	<div class="comment_body"><?= $comment->HTMLcomment(true); ?></div>

	<header class="comment_meta">
		<?= $commenter->htmlUsername() ?>
		<a href="<?= Link::comment($comment) ?>"><time class="timestamp"<?php if($ts) { ?> datetime="<?= date('c', $ts) ?>"<?php } ?>><?= $comment->timestamp() ?></time></a>
	</header>

	<?php
	$thevote = $comment->vote();
	if($thevote) {
		$numgood += ($thevote == 'this is good') ? 1 : 0;
		$numbad += ($thevote == 'this is bad') ? 1 : 0;
	}
	$tmbo = $comment->tmbo() == 1;
	$tiar = $comment->tiar() == 1;
	?>
	<?php if($thevote || $tmbo || $tiar) { ?>
	<footer class="comment_votes">
		<?php if($thevote) { ?>
		<span title="+<?= $numgood ?> -<?= $numbad ?>" class="vote <?= strpos($thevote, 'good') > 0 ? 'good' : 'bad' ?>"> [ <?= $thevote ?> ]</span>
		<?php } ?>
		<?php if($tmbo) { ?><span class="vote"> [ this might be offensive ]</span><?php } ?>
		<?php if($tiar) { ?><span class="vote"> [ this is a repost ]</span><?php } ?>
	</footer>
	<?php } ?>
</article>
