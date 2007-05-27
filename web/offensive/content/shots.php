<?

function body() {

	$price = "4.99";

?>

<div class="heading">shoits!</div>
<div class="bluebox" style="text-align:center">

	<img src="graphics/shotglass.jpg" style="border:4px solid #333366"/><br/>

	<p style="font-size:1.5em"><b>[ this is good ] shot glass. made from 100% awesome.</b></p>
	<p>(alcohol sold separately.)</p>

	<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
		<input type="hidden" name="cmd" value="_xclick">
		<input type="hidden" name="business" value="merch@thismight.be">
		<input type="hidden" name="item_name" value="[ this is good ] shotglass">
		<input type="hidden" name="item_number" value="3">
		<input type="hidden" name="amount" value="<?= $price ?>">
		<input type="hidden" name="return" value="http://thismight.be/offensive/?c=thanks">
		<input type="hidden" name="no_note" value="1">
		<input type="hidden" name="currency_code" value="USD">
		<input type="hidden" name="lc" value="US">
		<p><input type="text" name="quantity" value="1" style="width:3em;"> <b>@ $<?= $price ?> each.</b></p>
		<input type="submit" name="submit" value="gimmee!" style="padding:2em;"/>
<!--		<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but23.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!"> -->
	</form>
	<br/>

	</form>
	<br/>
	
</div>
<?
}
?>
