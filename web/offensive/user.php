<?php 
	session_start();
	require_once '../admin/mysqlConnectionInfo.php';
	
	$usrid = $_REQUEST['userid'];
	if( ! is_numeric( $usrid ) ) {
		mail( "ray@mysocalled.com", "[" . $_SERVER['REMOTE_ADDR'] . "] ATTEMPTED ATTACK?", requestDetail(), "From: offensive@themaxx.com (this might be offensive)\r\nPriority: urgent" );
		session_unset();
		header( "Location: ./" );
	}

	function requestDetail() {
		ob_start();
		var_dump( $_SERVER );
		$string = ob_get_contents();
		ob_end_clean();
		return $string;
	}

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
        "http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd">


<html>
<head>
	<title>themaxx.com : [ this might be offensive ]</title>
	<META NAME="ROBOTS" CONTENT="NOARCHIVE">
	<meta name="generator" content="BBEdit 6.0.2">
	<link rel="stylesheet" type="text/css" href="filepilestyle.css" />
	<link rel="stylesheet" type="text/css" href="/styles/oldskool.css"/>

	<script type="text/javascript" src="http://www.filepile.org/pub/remote_logged_in.php"></script>
	<script type="text/javascript" language="javascript" src="/bitsUtils.js"></script>

	<script type="text/javascript">
		
		function onLoadHandler() {
			setLinkTargets();
			var cookieValue = self.logged_in_fipi ? "true" : "false";
			document.cookie = "logged_in_fipi=" + cookieValue + ";path=/;expires=Monday, 01-Apr-15 23:12:40 GMT";
		}
	
	</script>
	
<style type="text/css">

	/* override definitions in oldskool.css to make main content area wider */	

	#content {
		margin-left:auto;
		margin-right:auto;
		text-align:left;
		line-height: 15px;
		width:771px;
	}
	
	#rightcol {
		width:584px;
		float:left;
		margin-right: auto;
		margin-left: 0px;
	}
	
</style>
</head>
<body>

 <?php include( $DOCUMENT_ROOT . "/includes/headerbuttons.txt" );?>

	<div id="content">
	
		<div id="titleimg"><a href="./"><img src="graphics/offensive.gif" alt="[ this might be offensive ]" id="offensive" width="285" height="37" border="0"></a></div>

	
		<div id="leftcol">
			
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">log in</div>
					<div class="bluebox">
						<?php include 'login.php'?>
					</div>
				<div class="blackbar"></div>
			</div>


			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">where art thou romeo?</div>
					<div class="bluebox">
						<?php include 'finduserform.php'?>
					</div>
				<div class="blackbar"></div>
			</div>
			
			<div class="contentbox">

				<div class="blackbar"></div>
					<div class="heading">bandwidth isn't free:</div>
					<div class="bluebox">
						<p>help keep this thing running. please make a small donation to help pay for bandwidth.<p>
						<div style="text-align:center">						
							<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
								<input type="hidden" name="cmd" value="_xclick">
								<input type="hidden" name="business" value="bandwidth@themaxx.com">

								<input type="hidden" name="item_name" value="[ this might be offensive ] bandwidth">
								<input type="hidden" name="no_shipping" value="1">
								<input type="hidden" name="no_note" value="1">
								<input type="hidden" name="currency_code" value="USD">
								<input type="hidden" name="tax" value="0">
								<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but04.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">
							</form>
							
						</div>
					</div>

				<div class="blackbar"></div>
			</div>
						
			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">about box:</div>
					<div class="bluebox">
						<p>the magical auto-updating images page. I did not create or choose or approve these files. It is highly likely that I have not even seen them. Some of it is porn. Some of it is disgusting. Some of it is not suitable for viewing by anyone at all. If you're not comfortable with such things, go elsewhere.</p>
					</div>

				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">web hosting provided by:</div>
					<div class="bluebox" style="text-align:center">
						<a href="http://www.dreamhost.com/rewards.cgi"><img src="/graphics/dreamhost.gif" alt="dreamhost" width="88" height="33" hspace="0" vspace="0"></a>
					</div>

				<div class="blackbar"></div>
			</div>


			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="heading">archives:</div>
				<div class="bluebox">
					<?php include( 'ziplist.php' ) ?>
				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
				<div class="bluebox" align="center">
					<form name="spawnbox">					
												<input type="checkbox" name="spawn"  onClick="toggleWindowPref(this.checked)"><a href="#" title="check this box and links will open in new windows." onClick="document.forms['spawnbox'].elements['spawn'].checked = !(document.forms['spawnbox'].elements['spawn'].checked); toggleWindowPref(document.forms['spawnbox'].elements['spawn'].checked); return false">spawn new windows?</a>
					</form>

				</div>			
				<div class="blackbar"></div>
			</div>

			<div class="contentbox">
				<div class="blackbar"></div>
					<div class="heading">contact:</div>
					<div class="bluebox">
						<a href="/contact/">email</a><br>

						aim: <a href="aim:goim?screenname=themaxxcom">themaxxcom</a><br>
					</div>
				<div class="blackbar"></div>
			</div>
			
		</div> <!-- end left column -->

<?
	$link = openDbConnection();
	
	$query_result = mysql_query('set @good=0,@bad=0');
	$sql = "SELECT max(users.username), max(users.account_status), max(if(vote='this is good',@good:=@good+1,@bad:=@bad+1)), max(@good), max(@bad)
		FROM offensive_comments, offensive_uploads, users
		WHERE offensive_uploads.userid ={$usrid} AND offensive_comments.fileid = offensive_uploads.id AND users.userid = offensive_uploads.userid AND vote
		";
	list($name, $status, $junk, $good_votes, $bad_votes) = mysql_fetch_array(mysql_query($sql));	
	
?>
		
		<div id="rightcol">
			<div class="contentbox">

				<div class="blackbar"></div>
					<div class="heading">
						<? echo $name ?> <? echo ($status == 'normal') ? " is welcome here." : " was acting retarded and was sent home." ?>
					</div>

					<div class="bluebox">
						

						<table width="100%" border="0" cellpadding="0" cellspacing="0">
							<tr>
								<td>
									<div class="piletitle"><?echo $name ?>'s contributions to society: (+<?php echo $good_votes ?> -<?php echo $bad_votes ?>)</div>
								</td>
							</tr>
							<tr>
						
								<td><img src="graphics/clear.gif" border="0" height="3" width="1"></td>
							</tr>
							
							<tr>
								<td valign="top">
								
<table style="width:100%">
									
<?php
	
	$sql = 'SELECT filename, username, timestamp, up.id AS fileid FROM users, offensive_uploads up WHERE users.userid = ' . $usrid . ' AND up.userid = users.userid ORDER BY timestamp DESC LIMIT 100';
	$result = mysql_query( $sql );

	$class = 'evenfile';
	while( $row = mysql_fetch_assoc( $result ) ) {
		$class = ($class == 'evenfile') ? 'oddfile' : 'evenfile';
		?><tr>
			<td class="<?php echo $class?>"><a href="pages/pic.php?id=<?php echo $row['fileid'] ?>" class="<?php echo $class?>"><?php echo $row['filename']?></td>
			<td class="<?php echo $class?>"><a href="comments.php?fileid=<?php echo $row['fileid'] ?>" class="<?php echo $class?>">comments</a></td>
			<td style="text-align:right" class="<?php echo $class?>"><?php echo $row['timestamp']?></td>
		</tr><?php
	
	}

?>
</table>

								</td>
							</tr>
						</table>
						
						
					</div>
				<div class="blackbar"></div>

			</div>
		</div>		
		
	</div>





</body>
</html>
