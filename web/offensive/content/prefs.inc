<?

require_once( 'admin/mysqlConnectionInfo.inc' );
if(!isset($link) || !$link) $link = openDbConnection();


	function start() {
		mustLogIn();
	}

	function head() {
?>
		<script type="text/javascript">
			function setKey( srcField, fieldId, e ) {
				var keycode = (e.which > 0) ? e.which : e.keyCode;
				if( keycode == 9 ) {
					return true;
				}
				if( keycode > 0 ) {
					var field = document.getElementById( fieldId );
					field.value = keycode;
					srcField.value = valueFor( keycode );
				}
			}

			function valueFor( key ) {
				switch( key ) {

					case 38:
						// up arrow
						return String.fromCharCode( 0x2191 );
					break;					

					case 37:
						// left arrow
						return String.fromCharCode( 0x2190 );
					break;
					
					case 39:
						// right arrow
						return String.fromCharCode( 0x2192 );
					break;

					case 40:
						// down arrow
						return String.fromCharCode( 0x2193 );
					break;					

					default:
						return String.fromCharCode( key );
					break;
				}
			}

		</script>

<?
	}
?>

<?
	function body() {
?>

	<div class="heading">prefs</div>
	<div class="bluebox">
		<form 
			action="<? echo $_SERVER['PHP_SELF']?>"
		>

			<input type="hidden" name="c" value="prefs"/>
			<? keyBindingsForm( $_SESSION['userid'] ) ?>
			<input type="submit" value="save"/>

		</form>
	</div>

<?
	}
	
	
	function keyBindingsForm( $uid ) {
		
		$sql = "SELECT * FROM preference_names_values WHERE value like '%_KEY' ORDER BY id";
		
		$result = tmbo_query( $sql );
		while( $row = mysql_fetch_assoc( $result ) ) {
			?>
				<?= $row['value'] ?>: <input type="text" onkeypress="setKey( this, '<?= $row['value'] ?>', event );" maxlength="1" value="&larr;" size="3"/>
				<input type="hidden" name="<?= $row['value'] ?>" id="<?= $row['value'] ?>" value="37"/><br/>
			<?
		}
	}
	
?>