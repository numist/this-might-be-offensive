<?
	function start() {
		mustLogIn();
	}


	function isValidPassword( $pw ) {
		return preg_match( "/[a-z0-9_\-\.]{5}/i", $pw );
	}

	function body() {
?>	
	<div class="heading">ooo! top secret codeword spy stuff!</div>
	<div class="bluebox">
<?
	
		if( array_key_exists("old", $_REQUEST) ) {
			$old = $_REQUEST['old'];
			$new = $_REQUEST['new'];
			$uid = $_SESSION['userid'];
			
			if( isValidPassword( $new ) ) {
				$encrypted_pw = sha1( $old );
				$query = "SELECT userid FROM users
							WHERE userid = $uid
							AND password = '$encrypted_pw'";
	
				$result = tmbo_query( $query );
				if( mysql_num_rows( $result ) == 1 ) {
					$encrypted_pw = sha1( $new );
					$sql = "UPDATE users SET password='$encrypted_pw' WHERE userid=$uid LIMIT 1";
					$result = tmbo_query( $sql );
					?>done.<br/><br/><?
				}
				else {
					?>nope.<?
				}
			}
			else {
				?>new password is invalid.<?
			}
		}

?>
		<form action="./" method="post">
			old password: <input type="password" name="old"/><br/>
			new password: <input type="password" name="new"/><br/>
			<input type="hidden" name="c" value="chpw"/>
			<input type="submit" value="make it so"/>
		</form>
	</div>
<?
	}

?>
