<?
	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

	require("offensive/data/keynav.inc");

	function start()
	{
		global $key_options;
		mustLogIn();
		
		if(array_key_exists("submit", $_POST)) {
			// check for post for keys
			$choices = array();
			for($n = 0; array_key_exists("key$n", $_POST) && array_key_exists("keycode$n", $_POST); $n++) {
				// filter out invalid options
				if($_POST["key$n"] == "noselect" || !array_key_exists($_POST["key$n"], $key_options)) {
					continue;
				}
				
				if(!array_key_exists($_POST["key$n"], $choices))
					$choices[$_POST["key$n"]] = array();
				$choices[$_POST["key$n"]][] = $_POST["keycode$n"];
			}
		
			$prefnames = array_keys($key_options);
			if(count($choices) > 0) {
				foreach($prefnames as $prefname) {
					if($prefname == "noselect") continue;
					
					if(array_key_exists($prefname, $choices)) {
						me()->setPref($prefname, serialize($choices[$prefname]));
					} else {
						me()->setPref($prefname, "");
					}
				}
			} else {
				foreach($prefnames as $prefname) {
					if($prefname == "noselect") continue;
					
					me()->setPref($prefname, "");
				}
			}
		}
		
		if(array_key_exists("revoke", $_REQUEST)) {
			core_destroytoken($_REQUEST["revoke"]);
			header("Location: /offensive/?c=settings");
			exit;
		}
	}

	function head()
	{
?>
	<script type="text/javascript" src="/offensive/js/jquery-1.2.6.min.js"></script>
	<script type="text/javascript" src="/offensive/js/settings.php"></script>
<?php
	}

	function body()
	{
		global $key_options, $key_defaults;

		?><div class="heading">
				set things up how you like 'em.
		</div><?
		
		global $activeTab;
		tabs();
?>

	<div class="heading">browsing filters:</div>
	<div class="bluebox">
		<p>use the nsfw and tmbo filters to block images that are not safe for work or potentially offensive.  <br />use the bad filter if you trust the judgement of your fellow maxxers.  10 [this is bad] votes and a ratio of 9 bad for every [this is good] causes the bad filter to activate.</p>
		<div style="text-align:center">
			<span><?
				if(me()->getPref("hide_nsfw") == 1) { ?>
					<a href="/offensive/setPref.php?p=hide_nsfw&v=">turn nsfw filter off</a>
				<? } else { ?>
					<a href="/offensive/setPref.php?p=hide_nsfw&v=1">turn nsfw filter on</a>
				<? } ?>
			</span>
						
			<span style="margin-left:48px;"><?
				if(me()->getPref("hide_tmbo") == 1) { ?>
					<a href="/offensive/setPref.php?p=hide_tmbo&v=">turn tmbo filter off</a>
				<? } else { ?>
					<a href="/offensive/setPref.php?p=hide_tmbo&v=1">turn tmbo filter on</a>
				<? } ?>
			</span>
			
			<span style="margin-left:48px;"><?
				if(me()->getPref("hide_bad") == 1) { ?>
					<a href="/offensive/setPref.php?p=hide_bad&v=">turn bad filter off</a>
				<? } else { ?>
					<a href="/offensive/setPref.php?p=hide_bad&v=1">turn bad filter on</a>
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
						<?foreach($key_options as $name => $desc) {
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
			
				<table style="margin-left: 140px;">
					<!-- this should be changed to PHP with data from the backend -->
					<!-- you can forget about the name/id, that'll be set on submit  -->
					<?/*LOOPY LOOP*/
					
					// has the user customized their key layout?
					$custom = false;

                	$prefnames = array_keys($key_options);

					// desc is unused, but required to get the key in a foreach loop.
					foreach($prefnames as $prefname) {
						$values = me()->getPref($prefname);
						
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
						foreach($key_defaults as $type => $codes) {
							foreach($codes as $code) {
								emitRow($type, $code);
							}
						}
					}
					?>
					<!-- end of data from database -->
				</table>
				<center>
					<div id="key_add">
						<a href="#">
							<span>add row</span>
						</a>
					</div>
					<input id="keynav_submit" type="submit" value="Save" name="submit">
					<p>to reset to default, delete all rows and set the last one to "<?= $key_options["noselect"] ?>".</p>
				</center>
			</form>
		</div>
	</div>

	<div class="heading">tokens:</div>
	<div class="bluebox">
		<p>RSS feeds, the web site, and external applications use tokens to log in as you. Here you can see all of your tokens, when they were issued, for whom they were issued, and when they were last used. You can also revoke them here.</p>

			<div align="center" style="padding:1em;">
			<?
				$tokens = core_gettokens();
				foreach($tokens as $token) {
			?>
			<table>
				<tr>
					<td colspan="2">application: <?= (strlen($token->issued_to()) > 64) ? substr($token->issued_to(), 0, 61)."…" : $token->issued_to() ?></td>
				</tr>
				<tr>
                    <td colspan="2">token: <?= $token->tokenid() ?><?= $token->readonly() ? " (read only)" : "" ?></td>
				</tr>
				<tr>
					<td>issued date: <?= $token->issue_date() ?></td>
					<td>last used: <?= $token->last_used() ?></td>
				</tr>
				<tr>
					<td colspan="2" style="text-align: center;"><a href="<?= $_SERVER["REQUEST_URI"] ?>&revoke=<?= $token->tokenid() ?>">revoke this token</a></td>
				</tr>
			</table>
			<? } ?>
			</div>
	</div>

<?
	}
	
	function emitRow($prefname, $keycode) {
		global $key_options;?>
		<tr>
			<td class="key_delete">
				<a href="#"><span>delete</span></a>
			</td>
			<td class="keytype">
				<select>
					<? foreach($key_options as $name => $desc) {
						?><option value="<?= $name ?>"<?= $name == $prefname ? " selected" : "" ?>><?= $desc ?></option><?
					} ?>
				</select>
			</td>
			<td><input size='16' class="keycode" id="" name="" value="<?= $keycode ?>" /></td>
			<td><span class="key_notification"></span></td>
			<td><input type="hidden" class="registered_key" value="<?= $keycode ?>" /></td>
		</tr>
	<? }
?>