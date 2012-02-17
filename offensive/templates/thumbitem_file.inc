<?

// sanity
if(!defined("TMBO") || !isset($upload)) {
	if(!defined("TMBO")) {
		set_include_path("../..");
		define("TMBO", true);
	}
	require("offensive/500.php");
}

?>
<li fileid="<?= $upload->id() ?>">
  <div class="thumbcontainer">
		<div>
			<a href="<?= Link::upload($upload) ?>"
			<?php if($upload->filtered()) { ?>
				onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
				onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
			<? } ?>
				title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>">
				<img name="th<?= $upload->id()?>"
						 src="<?= $upload->filtered() ? "/offensive/graphics/th-filtered.gif" : $upload->thumbURL() ?>">
			</a>
		</div>
		<div>
			<span class="score">
				<a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel(); ?></a>
			</span>
		</div>
  </div>
</li>