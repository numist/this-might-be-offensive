<?

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

?>
<a name="<?= $comment->id() ?>"></a>
<div class="entry u<? echo $commenter->id();
	if($comment->vote() == "this is good") echo " tig";
	if($comment->vote() == "this is bad") echo " tib";
	if($comment->tiar()) echo " tiar";
	if($comment->tmbo()) echo " tmbo";
	if(strlen($comment->text())) echo " comment"; ?>"
	style="<?= isset($css) ? $css : '' ?>">
		<span class='comment_body'><?= $comment->HTMLcomment(); ?></span>
		<br />
		<div class="timestamp"><a href="<?= Link::comment($comment) ?>"><?= $comment->timestamp() ?></a></div>

		&raquo; <?= $commenter->htmlUsername() ?>

		<?
		if( $comment->vote() ) {
			$thevote = $comment->vote();
			$numgood += ($thevote == 'this is good') ? 1 : 0;
			$numbad += ($thevote == 'this is bad') ? 1 : 0;
			echo "<span title=\"+$numgood -$numbad\" class='vote".(strpos($comment->vote(), 'good') > 0 ? " good" : " bad")."'> [ " . $thevote . " ]</span>";
		}

		if( $comment->tmbo() == 1 ) {
			?><span class="vote"> [ this might be offensive ]</span><?php
		}

		if( $comment->tiar() == 1 ) {
			?><span class="vote"> [ this is a repost ]</span><?php
		}

		?>
</div>
