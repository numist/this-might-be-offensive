<?

	require_once( "offensive/assets/activationFunctions.inc" );
	require_once( "offensive/assets/tabs.inc" );
	require_once( "offensive/assets/functions.inc" );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	require_once("offensive/assets/upload.inc");
	if(!isset($link) || !$link) $link = openDbConnection();

	function start() {
		global $readonly;
		
		global $me;
		mustLogIn();
		
		$me = new User($_SESSION["userid"]);

		if($readonly) {
			header("Location: ./");
		}
		
	}
	
	function body() {
		// TODO: uploadForm should move back out here.
		return upload();
	}

	function uploadForm($fileuploaded=false) {
		$uploadsRemaining = numUploadsRemaining( $_SESSION['userid'] );
		?>
			<div class="heading"><?
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

					if( $uploadsRemaining !== 0 || $_SESSION['status'] == "admin" ) { 
						$remaining = $_SESSION['status'] == 'admin' ? '∞' : $uploadsRemaining;

				?>

						<p>You have <? echo $remaining ?> upload<? echo $remaining === 1 ? "" : "s"?> left.</p>

						<p>If you haven't already, please take a look at <a href="./?c=faq">the rules</a> before uploading.</p>



					<form method="post"
							action="<?php echo $_SERVER['PHP_SELF']?>"
							enctype="multipart/form-data"
							onsubmit="return validate()"
					>

							<table border="0" cellpadding="4" cellspacing="0" style="text-align:left;margin-left:auto;margin-right:auto">
								<tr>
									<td style="text-align:right"><label for="image">file:</label></td>
									<td><input type="file" name="image" id="image" onchange="setFileName( this )"/></td>
								</tr>	
								<tr>
									<td></td>
									<td><input type="checkbox" id="nsfw" name="nsfw" value="1"/><label for="nsfw">[ nsfw ]</label></td>
								</tr>
								<tr>
									<td></td>
									<td><input type="checkbox" id="tmbo" name="tmbo" value="1"/><label for="tmbo">[ this might be offensive ]</label></td>
								</tr>			

								<tr>
									<td></td>
									<td>
										<input type="hidden" name="filename" value=""/>
										<input type="hidden" name="c" value="upload"/>
										<input type="submit" value="upload"/>
									</td>
								</tr>	
							</table>

					</form>

				<?php } else { ?>

						<p>Save some for later, man.</p>
						<p><a href="./">index</a></p>

				<?php } ?>
			</div>		

		</div>

		<?

	}


?>