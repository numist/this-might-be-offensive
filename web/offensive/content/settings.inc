<?
	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require("offensive/data/keynav.inc");

	function start()
	{
		global $options, $me;
		
		mustLogIn();
		if(!isset($me)) $me = new User($_SESSION["userid"]);
		
		if(array_key_exists("submit", $_POST)) {
			// check for post for keys
			$n = 0;
			$choices = array();
			
			while(array_key_exists("key$n", $_POST) && array_key_exists("keycode$n", $_POST)) {
				// filter out invalid options
				if($_POST["key$n"] == "noselect" || !array_key_exists($_POST["key$n"], $options)) {
					$n++;
					continue;
				}
				
				if(!array_key_exists($_POST["key$n"], $choices))
					$choices[$_POST["key$n"]] = array();
				$choices[$_POST["key$n"]][] = $_POST["keycode$n"];
				$n++;
			}
		
			if(count($choices) > 0) {
				foreach($options as $prefname => $val) {
					if($prefname == "noselect") continue;
					
					if(array_key_exists($prefname, $choices)) {
						$me->setPref($prefname, serialize($choices[$prefname]));
					} else {
						$me->setPref($prefname, "");
					}
				}
			} else {
				foreach($options as $prefname => $val) {
					if($prefname == "noselect") continue;
					
					$me->setPref($prefname, "");
				}
			}
		}
	}

	function head()
	{
?>
	<link rel="stylesheet" type="text/css" href="/offensive/css/settings.css"/>
	<script type="text/javascript" src="/offensive/js/jquery-1.2.6.min.js"></script>
	<!-- XXX: settings2? -->
	<script type="text/javascript" src="/offensive/js/settings.js"></script>
<?php
	}

	function body()
	{
		global $me, $options;

		?><div class="heading">
				set things up how you like 'em.
		</div><?
		
		global $activeTab;
		tabs();
?>

	<div class="heading">browsing filters:</div>
	<div class="bluebox">
		<p>use these to filter out images tagged as nsfw or tmbo.</p>
		<div style="text-align:center">
			<span><?
				if($me->getPref("hide_nsfw") == 1) { ?>
					<a href="/offensive/setPref.php?p=hide_nsfw&v=">turn nsfw filter off</a>
				<? } else { ?>
					<a href="/offensive/setPref.php?p=hide_nsfw&v=1">turn nsfw filter on</a>
				<? } ?>
			</span>
						
			<span style="margin-left:48px;"><?
				if($me->getPref("hide_tmbo") == 1) { ?>
					<a href="/offensive/setPref.php?p=hide_tmbo&v=">turn tmbo filter off</a>
				<? } else { ?>
					<a href="/offensive/setPref.php?p=hide_tmbo&v=1">turn tmbo filter on</a>
				<? } ?>
			</span>
		</div>
	</div>

	<div class="heading">keyboard navigation:</div>
	<div class="bluebox">
		<!-- template for each row -->
		<textarea style="display:none;" id="template">
			<tr>
				<td class="key_delete">
					<a href="#"><span>delete</span></a>
				</td>
				<td class='keytype'>
					<select>
						<?foreach($options as $name => $desc) {
							?><option value="<?= $name ?>"><?= $desc ?></option><?
						}?>
					</select>
				</td>
				<td><input size='8' class="keycode" min="1" id="" name="" /></td>
				<td><span class="key_notification"></span></td>
				<td><input type="hidden" class="registered_key"></span></td>
			</tr>
		</textarea>
		<div id="keynav">
			<form id="keynav_form" action="/offensive/?c=settings" method="post">
				<p>choose your own special keyboard nav settings. pick an action from the dropdown, choose a key to make it happen. </p><p>if you see a number in the box instead of a letter, the number is the keycode for the pressed key, which sometimes varies based on keyboard layout.</p><p>if the box has a letter, but the letter is wrong, please let someone know.</p><p>if you try to use the same key for multiple things, I can't help you.  it probably won't work correctly, so don't bother.</p>
			
				<center><table>
					<!-- this should be changed to PHP with data from the backend -->
					<!-- you can forget about the name/id, that'll be set on submit  -->
					<?/*LOOPY LOOP*/
					
					// has the user customized their key layout?
					$custom = false;
                	
					// desc is unused, but required to get the key in a foreach loop.
					foreach($options as $prefname => $desc) {
						$values = $me->getPref($prefname);
						
						// possible false values here are false or "", so not checking types.
						if($values != false) {
							$values = unserialize($values);
							foreach($values as $value) {
								emitRow($prefname, $value);
								$custom = true;
							}
						}
					}
					if(!$custom) {
						emitRow("key_next", 37);		// ←
						emitRow("key_next", 178);       // Wii ←
						
						emitRow("key_prev", 39);		// →
						emitRow("key_prev", 177);       // Wii →
						
						emitRow("key_comments", 40);		// ↓
						emitRow("key_comments", 176);    // Wii ↓
						
						emitRow("key_index", 38);		// ↑
						emitRow("key_index", 175);      // Wii ↑
						
						emitRow("key_good", 61);		// +
						emitRow("key_good", 107);       // + (numpad)
						emitRow("key_good", 187);       // =
						emitRow("key_good", 174);       // Wii +
						
						emitRow("key_bad", 109);		// - (numpad)
						emitRow("key_bad", 189);        // -
						emitRow("key_bad", 170);        // Wii -
						
						emitRow("key_quick", 81);		// q
						
						emitRow("key_escape", 27);		// Esc
						
						emitRow("key_random", 191);		// ?
					}
					?>
					<!-- end of data from database -->
				</table>
				<div id="key_add">
					<a href="#">
						<span>add row</span>
					</a>
				</div>
				<input id="keynav_submit" type="submit" value="Save" name="submit">
				<p>to reset to default, delete all rows and set the last one to "<?= $options["noselect"] ?>".</p>
				</center>
			</form>
		</div>
	</div>
<?
	}
	
	function emitRow($prefname, $keycode) {
		global $options;?>
		<tr>
			<td class="key_delete">
				<a href="#"><span>delete</span></a>
			</td>
			<td class="keytype">
				<select>
					<? foreach($options as $name => $desc) {
						?><option value="<?= $name ?>"<?= $name == $prefname ? " selected" : "" ?>><?= $desc ?></option><?
					} ?>
				</select>
			</td>
			<td><input size='8' class="keycode" min="1" id="" name="" value="<?= $keycode ?>" /></td>
			<td><span class="key_notification"></span></td>
			<td><input type="hidden" class="registered_key" value="<?= $keycode ?>" /></td>
		</tr>
	<? }
?>