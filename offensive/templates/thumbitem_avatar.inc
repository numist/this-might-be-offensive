<li fileid="<?= $upload->id() ?>">
	<div class="thumbcontainer">
		<div><a href="<?= Link::upload($upload) ?>"
			<? if($upload->filtered()) { ?>
			onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
			onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
			<? } ?>
		><img name="th<?= $upload->id()?>"
			src="<?= $upload->filtered()
				? "/offensive/graphics/th-filtered.gif"
				: $upload->thumbURL() ?>"
			>
		</a></div>
		<div class="score">
			<?= $upload->uploader()->htmlUsername() ?>
			<a href="<?= Link::thread($upload) ?>">[c]</a>
		</div>
	</div>
</li>