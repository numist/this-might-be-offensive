<?
function start() {
	mustLogIn();
}

function body() {
?>

	<div class="heading">
		subscribe:
	</div>

	<div class="bluebox">

		<div style="text-align:center">
			<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
				<input type="hidden" name="cmd" value="_xclick-subscriptions">
				<input type="hidden" name="business" value="bandwidth@thismight.be">
				<input type="hidden" name="item_name" value="[ tmbo ]">
				<input type="hidden" name="no_shipping" value="1">
				<input type="hidden" name="no_note" value="1">
				<input type="hidden" name="currency_code" value="USD">
				<input type="hidden" name="lc" value="US">
				<input type="hidden" name="bn" value="PP-SubscriptionsBF">
				<input type="hidden" name="p3" value="1">
				<input type="hidden" name="t3" value="M">
				<input type="hidden" name="src" value="1">
				<input type="hidden" name="sra" value="1">
				monthly	amount: <input type="text" name="a3" size="6" value="15.00">
				<br/><br/>
				<input type="image"
					src="graphics/paypal_subscribe.gif"
					border="0"
					name="submit"
					alt="Make payments with PayPal - it's fast, free and secure!" />
			</form>
		</div>
	</div>


<?
}
?>