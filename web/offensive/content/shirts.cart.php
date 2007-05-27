<?

function body() {
?>

<div class="heading">shoits!</div>
<div class="bluebox">

	<img src="graphics/shirts.jpg" style="border:4px solid #333366"/><br/>

	<p><b>$14.00 [ this might be offensive ] tee</b>. navy hanes beefy-tee. 100% cotton.</p>

	<form target="paypal" action="https://www.paypal.com/cgi-bin/webscr" method="post">
		<table>
			<tr>
				<td><input type="hidden" name="on0" value="Size">Size</td>
				<td>
					<select name="os0">
						<option value="S">S</option>
						<option value="M">M</option>
						<option value="L" SELECTED="SELECTED">L</option>
					</select>
				</td>
			</tr>
		</table>
		
		<input type="image" src="https://www.paypal.com//en_US/i/btn/sc-but-03.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
		<input type="hidden" name="add" value="1">
		<input type="hidden" name="cmd" value="_cart">
		<input type="hidden" name="business" value="shirts@themaxx.com">
		<input type="hidden" name="item_name" value="[ this might be offensive ] t-shirt">
		<input type="hidden" name="amount" value="14.00">
		<input type="hidden" name="return" value="http://themaxx.com/offensive/?c=thanks">
		<input type="hidden" name="no_note" value="1">
		<input type="hidden" name="currency_code" value="USD">
		<input type="hidden" name="lc" value="US">
	</form>
	<br/>
	<form target="paypal" action="https://www.paypal.com/cgi-bin/webscr" method="post">
		<input type="hidden" name="cmd" value="_cart">
		<input type="hidden" name="business" value="shirts@themaxx.com">
		<input type="image" src="https://www.paypal.com/en_US/i/btn/view_cart.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
		<input type="hidden" name="display" value="1">
	</form>
	
</div>
<?
}
?>

