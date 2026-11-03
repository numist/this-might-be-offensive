<?php

	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/tabs.inc" );
	require_once( "offensive/assets/functions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	require_once("offensive/assets/upload.inc");
	if(!isset($link) || !$link) $link = openDbConnection();

	function start() {
		mustLogIn();

		if(TMBO::readonly()) {
			header("Location: ./");
		}
		
	}
	
	function head() {
		CSS::add("/styles/upload.css");
		JS::add("/offensive/js/upload.js");
	}

	function body() {
		return upload();
	}

	function uploadForm($fileuploaded=false, $avatar=false) {
		$uploadsRemaining = numUploadsRemaining();
		?>
		<div class="contentbox">
			<div class="blackbar"></div>
			<div class="heading"><?php
				if($fileuploaded && $uploadsRemaining !== 0) {
					echo "thank you, sir, may i have another?";
				} else if ($uploadsRemaining !== 0) {
					echo "gimme.";
				} else {
					echo "wait.";
				} ?></div>

		<div class="bluebox">

			<div style="text-align:center">

				<?php

					if( $uploadsRemaining !== 0 || me()->status() == "admin" ) {
						$remaining = me()->status() == 'admin' ? '∞' : $uploadsRemaining;

				?>

						<!--<p>You have <?php echo $remaining ?> upload<?php echo $remaining === 1 ? "" : "s"?> left.</p>-->

						<p>If you haven't already, please take a look at <a href="<?= Link::content("faq") ?>">the rules</a> before uploading.</p>



					<form id="uploadform" method="post"
					      action="<?php echo $_SERVER['PHP_SELF']?>"
					      enctype="multipart/form-data">
						<!-- the file input shows only its button (styles/upload.css); once a
						     file is picked, the title field and extension take the place of
						     the browser's file-name readout -->
						<input type="file" name="image" id="image" required aria-label="file" />
						<input type="text" name="filename" id="filename" maxlength="120"
						       aria-label="title" aria-describedby="extension" placeholder="name it."
						       autocomplete="off" enterkeyhint="send" />
						<span id="extension" title="the file extension is added for you">.ext</span>

						<fieldset>
							<label><input type="checkbox" name="nsfw" value="1"/> [ nsfw ]</label>
							<label><input type="checkbox" name="tmbo" value="1"/> [ this might be offensive ]</label>
						</fieldset>

<?php if($avatar) {?>
						<input type="hidden" name="avatar" value=""/>
<?php }?>
						<input type="hidden" name="c" value="upload"/>
						<input type="submit" value="upload"/>
					</form>

				<?php } else { ?>

						<p>Save some for later, man.</p>
						<p><a href="<?= Link::mainpage() ?>">index</a></p>

				<?php } ?>
			</div>		
		</div>
		<div class="blackbar"></div>
	</div>
	<?php
	}
?>