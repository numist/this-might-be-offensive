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
				title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>">
				<img name="th<?= $upload->id()?>"
						 class="<?php
						 if ($upload->is_nsfw()) echo "nsfw ";
						 if ($upload->is_tmbo()) echo "tmbo ";
						 if ($upload->is_bad()) echo "bad ";
						 ?>"
						 src="<?= $upload->thumbURL() ?>">
			</a>
		</div>
		<div>
			<span class="score">
				<a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"><?= $upload->commentLabel(); ?></a>
			</span>
		</div>
  </div>
</li>
