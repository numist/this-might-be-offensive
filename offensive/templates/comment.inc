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

require_once("offensive/templates/comment_parts.inc");

?>

<article id="<?= $comment->id() ?>" class="entry u<?php echo $commenter->id();
	if($comment->vote() == "this is good") echo " tig";
	if($comment->vote() == "this is bad") echo " tib";
	if($comment->tiar()) echo " tiar";
	if($comment->tmbo()) echo " tmbo";
	if(strlen($comment->text())) echo " comment"; ?>"
	<?php if(isset($css) && $css !== '') { ?>style="<?= $css ?>"<?php } ?>>

	<div class="comment_body"><?= $comment->HTMLcomment(true); ?></div>

	<?php
		commentVotes($comment, $numgood, $numbad);
		commentMeta($comment);
	?>

</article>

