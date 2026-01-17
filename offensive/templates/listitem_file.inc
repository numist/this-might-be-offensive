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
<li data-fileid="<?= $upload->id() ?>">
  <div class="col<?= $row_class ?>">
    <a
      href="<?= Link::upload($upload) ?>"
      title="uploaded by <?= htmlEscape($upload->uploader()->username()) ?>"
    >
      <?= $upload->htmlFilename() ?>
    </a>
    <? if(!file_exists($upload->file())) { echo "(expired)"; } ?>
  </div>
  <div class="col<?= $row_class ?>">
    <span class="score">
      <a href="<?= Link::thread($upload) ?>" title="<?= $upload->timestamp() ?>"
        ><?= $upload->commentLabel(); ?></a
      >
    </span>
  </div>
</li>
