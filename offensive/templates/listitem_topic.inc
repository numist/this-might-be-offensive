<?

// sanity
if(!defined("TMBO") || !isset($upload)) {
	if(!defined("TMBO")) {
		set_include_path("../..");
		define("TMBO", true);
	}
	require("offensive/500.php");
}

// with class or without?
if(isset($css) && substr($css, 0, 1) != " ") {
	$row_class = " ".$css;
} else {
	assert('!isset($css)');
	$row_class = "";
}

?>
<li fileid="<?= $upload->id() ?>">
	<div class="col col1<?= $row_class ?>">
		<a href="<?= Link::upload($upload) ?>" title="started by <?= htmlEscape($upload->uploader()->username()) ?>">
			<?= $upload->htmlFilename() ?>
		</a>
	</div>
	<div class="col col2<?= $row_class ?>">
		<span class="score">
			<a href="<?= Link::thread($upload) ?>">(<?=$upload->commentLabel(false);?>)</a>
		</span>
	</div>
	<div class="col col3<?= $row_class ?>">
			<?= $upload->last_active() ?>
	</div>
</li>