<?
	require_once( '../admin/mysqlConnectionInfo.php' );
	require_once( 'tabs.php' );	
	
	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			header( "Location: ./?c=mustLogIn" );
		}
	}

	function body() {
		global $view;
		
		$view = isset( $_REQUEST['t'] ) ? mysql_real_escape_string( $_REQUEST['t'] ) : 'yearbook';	
?>
	<div class="heading">the beautiful people.</div>
<?
		tabs();
				
?>
		
		<div style="padding-top:8px;background:#ccccff;background-image:url( graphics/subtab_bg.gif );background-position:top left;background-repeat:repeat-x">
			<div class="<?= cssForUserView( 'yearbook' ) ?>"><a href="./?c=users&t=yearbook">yearbook</a></div>
			<div class="<?= cssForUserView( 'map' ) ?>"><a href="./?c=users&t=map">map</a></div>
			<div class="<?= cssForUserView( 'invite' ) ?>"><a href="./?c=users&t=invite">invite</a></div>
			<div class="tabspacer" style="background:none">&nbsp;</div>
		</div>

		<div class="bluebox">
			<? yearbook(); ?>
		</div>
<?
	}
	
	function cssForUserView( $viewname ) {
		global $view;
		return ($view == $viewname) ? 'tabon' : 'taboff';
	}

	function yearbook() {

?>

<table width="100%" class="thumbnails">

<?

		$numPerPage = 100;
		$page = $_REQUEST['p'];
		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT users.userid, username, account_status, filename
					FROM users, offensive_uploads up
					WHERE users.userid=up.userid
						AND up.type='avatar'
						AND account_status = 'normal'
						AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up where type='avatar' AND userid=users.userid)
					ORDER BY username, up.id DESC";

		$result = mysql_query($sql);

		echo mysql_error();

		$THUMBS_PER_ROW = 4;
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
		
		
			if( (($count++) % $THUMBS_PER_ROW) == 0 ) {
				$css = ($css == "even_row") ? "odd_row" : "even_row";
				?><tr class="<?= $css ?>"><?
			}
		
			?>
				
					<td>
						<?
							$thumbnail = file_exists( "images/users/thumbs/th-" . $row['filename'] ) ? "th-" . $row['filename'] : "th-unavailable.gif"
						?>
						<a href="images/users/<?= $row['filename'] ?>"><img src="images/users/thumbs/<?= $thumbnail ?>"/></a>
						<br/>
						<a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?></a>
					</td>

			<?
			
			if( (($count) % $THUMBS_PER_ROW) == 0 ) {
				?></tr><?
			}
		}

		if( ($count % $THUMBS_PER_ROW) != 0 ) {
			?></tr><?
		}
		?>

	</table>


<?
	}
?>
