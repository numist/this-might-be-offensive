<?

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once( 'offensive/assets/functions.inc' );

	function start() {
		global $link;

		mustLogIn();

		if(!isset($link) || !$link) $link = openDbConnection();
		
		if( array_key_exists("invite", $_REQUEST) &&
		    isValidEmail( $_REQUEST['invite'] ) && 
		    canInvite( $_SESSION['userid'] ) ) {
			sendInvitation( $_REQUEST['invite'] );
			header( "Location: " . $_SERVER['PHP_SELF'] . "?c=referral&s=1" );
			exit;
		}
	}

	function isValidEmail( $email ) {
		$valid = preg_match( '/[a-zA-Z0-9-_\.]+\@[a-zA-Z0-9-_\.]+\.[a-zA-Z0-9-_\.]+/', $email ) > 0
					&& strpos( $email, "mailinator.com" ) == false;
		return $valid;
	}

	function canInvite( $uid ) {
		global $link;
		
		$sql = "select (created < date_sub( now(), INTERVAL 1 MONTH )) as canInvite
					FROM users where userid=$uid";
		if(!isset($link) || !$link) $link = openDbConnection();
		$result = tmbo_query( $sql );
		$row = mysql_fetch_assoc( $result );

		return $row['canInvite'] == 1;
	}

	function sendInvitation( $email ) {

		$username = $_SESSION['username'];
		$referralCode = generateReferralCode( $_SESSION['userid'], $email );
	
		$message = "

$username has invited you to join [ this might be offensive ].

Go to http://thismight.be/offensive/registr.php to register.

Referral code:   $referralCode

You must provide a valid email address (activation instructions will be sent there), and the referral code will only work once.

Enjoy.
";
	
		mail( $email, "[ this might be offensive ] account invitation", "$message", "From: offensive@thismight.be (this might be offensive)");
	
	}
	
	function generateReferralCode( $uid, $email ) {
		
		// fairly arbitrary generation of a pseudo-random string.
		$referral_code = md5( "$uid:" . time() . ":$uid" );
		
		$email = sqlEscape( $email );

		$sql = "insert into referrals ( userid, referral_code, email )
					values( $uid, '$referral_code', '".sqlEscape($email)."' );
		";
		
		tmbo_query( $sql );

		return $referral_code;
	}

	function showReferrals( $uid ) {

		$sql = "select * from referrals where userid=$uid";
		$result = tmbo_query( $sql );
		while( $row = mysql_fetch_assoc( $result ) ) {
			$nextStyle = ($nextStyle == "odd_row") ? "even_row" : "odd_row";
			?>
				<div class="entry <?= $nextStyle ?>"><?= $row['referral_code'] ?></div>
			<?
		}
		
		return mysql_num_rows( $result );
		
	}
	
	function body() {
?>

	<div class="heading">put your peeps on the list.</div>
	<? tabs(); ?>
	<div class="bluebox">

		<?
			if( array_key_exists("s", $_REQUEST) && 
			    $_REQUEST['s'] == 1 ) {
		?>
			<div class="entry even_row">
				Invitation sent.
			</div>
		<?
			}
		?>

		<div class="entry odd_row">
			<?
				if( canInvite( $_SESSION['userid'] ) ) {
			?>
			<p>enter the email address of a friend below to send them an account invitation.<br/>
			please use it wisely.</p>

			<p>
				<form action="<?= $_SERVER['PHP_SELF'] ?>">
					<input type="hidden" name="c" value="referral"/>
					<input type="text" name="invite"/>
					<input type="submit" value="this dude is cool. i promise."/>
				</form>
			</p>
			<?
				}
				else {
			?>
					<p>You have no invitations to send. We need to check you out and sniff your butt for 1 month before we let your friends in.</p>
			<?
				}
			?>
		</div>

	</div>
<?
	
	}
	

?>