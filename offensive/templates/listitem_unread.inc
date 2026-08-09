<?

// sanity
if(!defined("TMBO") || !isset($comment)) {
	if(!defined("TMBO")) {
		set_include_path("../..");
		define("TMBO", true);
	}
	require("offensive/500.php");
}
?>

<li>
    <div class="clipper"><a id="unread<?= $comment->upload()->id()?>" class="<?= isset($css) ? htmlEscape($css) : '' ?>" href="<?= Link::comment($comment) ?>"><?= $upload->htmlFilename() ?></a></div>
</li>
