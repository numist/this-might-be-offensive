<?
require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();

function start() {
	mustLogIn();
}

function body() {
?>
<div class="contentbox">
	<div class="blackbar"></div>
	<div class="heading">
		rules and stuff:
	</div>

	<div class="bluebox">
	
		<div>
			There's been a bit of confusion lately about what the rules are around here, so in the interest of clarity we've compiled the following list:
			<ol>
				<li>Don't be retarded.</li>	
			</ol>
			
			Please refer to this list when you have doubts about whether something you're about to do is inappropriate. Failure to observe these rules may result in account termination.
	
		</div>
	
	</div>
	
	<div class="heading">
		voting:
	</div>
	
	<div class="bluebox">
	
			We all agree that tsunamis, war, car crashes and mariah carey are bad. It's a question of whether you want to see the pics. Vote [this is good] to praise the uploader. Vote [this is bad] to discourage.
	</div>

	<a href="#uploading" name="uploading"></a>
	<div class="heading">
		uploading:
	</div>
	
	
	<div class="bluebox">
		<p>We like your cat. Really we do. But we just can't take all 74 pictures of cuddly at once; warm and fuzzy in such a high dosage quickly becomes hot and hairy, and then spontaneous human combustion. We don't want that. So there's a limit on how much you can upload.</p>
		
		<p>The upload limit is based on the number of good and bad votes you've received. The higher your good:bad ratio, the more files you're allowed to upload in a 24 hour period. (Just because you can doesn't mean you should.)</p>

	</div>

	<div class="blackbar"></div>
</div>

<?
}
?>