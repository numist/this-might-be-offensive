<?

function body() {

	$price = "14.00";

?>

<div class="heading">shoits!</div>
<div class="bluebox">

	<img src="graphics/shirts.jpg" style="border:4px solid #333366"/><br/>

	<p><b>$<?= $price ?> [ this might be offensive ] tee</b>. navy hanes beefy-tee. 100% cotton.</p>

	<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
		<table>
			<tr>
				<td><input type="hidden" name="on0" value="Size"/>Size</td>
				<td>
					<select name="os0">
						<option value="S">S</option>
						<option value="M" disabled="disabled">M (sold out)</option>
						<option value="L" disabled="disabled">L (sold out)</option>
						<option value="XL" disabled="disabled">XL (sold out)</option>
						<option value="XXL" SELECTED="SELECTED">XXL</option>						
					</select>
				</td>
			</tr>
		</table>

		<input type="hidden" name="cmd" value="_xclick">
		<input type="hidden" name="business" value="shirts@themaxx.com">
		<input type="hidden" name="item_name" value="[ this might be offensive ] tee">
		<input type="hidden" name="item_number" value="1">
		<input type="hidden" name="amount" value="<?= $price ?>">
		<input type="hidden" name="return" value="http://themaxx.com/offensive/?c=thanks">
		<input type="hidden" name="no_note" value="1">
		<input type="hidden" name="currency_code" value="USD">
		<input type="hidden" name="lc" value="US">

		<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but23.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">

	</form>
	<br/>
	
		<img src="graphics/hugmeshirt.jpg" width="500" height="375" style="border:4px solid #333366"/><br/>

	<p><b>$<?= $price ?> "hug me and descend into madness" tee</b>. navy hanes beefy-tee. 100% cotton.</p>

	<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
		<table>
			<tr>
				<td><input type="hidden" name="on0" value="Size"/>Size</td>
				<td>
					<select name="os0">
						<option value="S" disabled="disabled">S (sold out)</option>
						<option value="M" disabled="disabled">M (sold out)</option>
						<option value="L" disabled="disabled">L (sold out)</option>
						<option value="XL" SELECTED="SELECTED">XL</option>
						<option value="XXL">XXL</option>						
					</select>
				</td>
			</tr>
		</table>

		<input type="hidden" name="cmd" value="_xclick">
		<input type="hidden" name="business" value="shirts@themaxx.com">
		<input type="hidden" name="item_name" value="hug me tee">
		<input type="hidden" name="item_number" value="2">
		<input type="hidden" name="amount" value="<?= $price ?>">
		<input type="hidden" name="return" value="http://themaxx.com/offensive/?c=thanks">
		<input type="hidden" name="no_note" value="1">
		<input type="hidden" name="currency_code" value="USD">
		<input type="hidden" name="lc" value="US">

		<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but23.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">

	</form>
	<br/>
	
</div>
<?
}
?>
