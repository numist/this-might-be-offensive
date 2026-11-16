<?php

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
  <a href="<?= Link::thread($upload) ?>"><?=$upload->commentLabel(false);?></a>
  <a
    href="<?= Link::upload($upload) ?>"
    title="started by <?= htmlEscape($upload->uploader()->username()) ?>"
  >
    <?= $upload->htmlFilename() ?>
  </a>

<?php
  $last_commenter = $upload->last_commenter();
  if($last_commenter !== null) {
    $when = $upload->last_active();
    $activity = htmlEscape($last_commenter)." commented ".time_ago($when)." ago";
  } else {
    $when = $upload->timestamp();
    $activity = htmlEscape($upload->uploader()->username())." started it ".time_ago($when)." ago";
  }
?>
  <time datetime="<?= $when ?>"><?= $activity ?></time>
</li>
