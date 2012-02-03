<?php

set_include_path("..");
require("offensive/assets/header.inc");
// Include, and check we've got a connection to the database.
require_once('admin/mysqlConnectionInfo.inc');
if(!isset($link) || !$link) $link = openDbConnection();
require_once('offensive/assets/functions.inc');
mustLogIn();

require_once("offensive/assets/classes.inc");

$p = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0 ;

function poast($comment, $image, $upload) {
	$comment = new Comment($comment['commentid']);
	if(!me()->squelched($comment->commenter()->id())) {
		$com = $comment->HTMLcomment();
		$com = explode("\n", $com);
		if(count($com) <= 10) { 
			$com = implode("<br />", $com);
			echo $com;
		} else {
			for($i = 0; $i < 10; $i++) {
				echo $com[$i] . "<br />";
			}?>
				<blockquote>
					<span class="abbr">Comment too long. Click <a href="/offensive/?c=comments&fileid=<?= $upload->id() ?>#<?= $comment->id() ?>">here</a> to view the full text.</span>
				</blockquote>
			<?
		}
	}
}

?><html><head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8">
<meta name="robots" content="noarchive"/>
<meta name="description" content="/tmbo/ is the home of themaxx, it is where people go."/>
<meta name="keywords" content="imageboard,anonymous,random,/tmbo/"/>
<link rel="stylesheet" type="text/css" href="yotsuba.7.css" title="Yotsuba">
<link rel="alternate stylesheet" type="text/css" href="yotsublue.7.css" title="Yotsuba B">
<link rel="alternate stylesheet" type="text/css" href="futaba.7.css" title="Futaba">
<link rel="alternate stylesheet" type="text/css" href="burichan.7.css" title="Burichan">
<title>/tmbo/ - This Might Be Offensive</title>

<script type="text/javascript">
function setActiveStyleSheet(title) {
  var i, a, main;
  for(i=0; (a = document.getElementsByTagName("link")[i]); i++) {
    if(a.getAttribute("rel").indexOf("style") != -1 && a.getAttribute("title")) {
      a.disabled = true;
      if(a.getAttribute("title") == title) a.disabled = false;
    }
  }
}

/* image rollover stuff */
function changesrc(a,im)
{
	x = eval("document."+a);
	x.src=im;
}
</script>

<? include_once("analytics.inc"); ?>
</head>
<body bgcolor="#FFFFEE" text="#800000" link="#0000EE" vlink="#0000EE">
<div id="header"><span id="navtop">

[
<a href="/offensive/?c=user&userid=499" title="asshat">a</a> /
<a href="/offensive/?c=discussions" title="Message Board">d</a> /
<a href="/offensive/?c=hof" title="Hall of Fame">h</a> /
<a href="/offensive/?c=referral" title="Invite a Friend">i</a> /
<a href="/offensive/map/gmap.php" title="World Map">m</a> /
<a href="/offensive/?c=faq" title="Rules & Stuff">r</a> /
<a href="/offensive/?c=search" title="Search">s</a> /
<a href="/offensive/?c=yearbook" title="Yearbook">y</a>
]

[<a href="/offensive/?c=changeblog" title="tmbo Status">status</a>]

</span><span id="navtopr">[<a href="/offensive/?c=discussions" target="_top">Home</a>]</span></div><br/>
<center><div class="logo">
<img width=300 height=100 src="http://static.4chan.org/dontblockthis/title/batou01.gif"><br>
<font size=5>
<b><SPAN>/tmbo/ - This Might be Offensive</SPAN></b></font><br><font size=1>The stories and information posted here are artistic works of fiction and falsehood.<br />Only a fool would take anything posted here as fact.</font></div></center>
<hr width="90%" size=1>
<div style='position:relative'></div><div align="center" class="postarea">

<form method="post"
	action="/offensive/?c=upload"
	enctype="multipart/form-data">
	<table cellpadding=1 cellspacing=1>
		<tr>
			<td></td>
			<td class="postblock" align="left">
				<b>File</b>
			</td>
			<td colspan="2">
				<input type="file" name="image" size="35">
			</td>
		</tr><tr>
			<td></td>
			<td class="postblock" align="left">
				<b>[nsfw]</b>
			</td>
			<td colspan="2">
				<input class="inputtext" type="checkbox" id="nsfw" name="nsfw" value="1"/>
				<small>(I guess it depends where you work...)</small>
</td>
		</tr><tr>
			<td></td>
			<td class="postblock" align="left">
				<b>[tmbo]</b>
			</td>
			<td>
				<input class="inputtext" type="checkbox" id="tmbo" name="tmbo" value="1"/>
				<input type="hidden" name="filename" value=""/>
				<input type="hidden" name="c" value="upload"/>
			</td><td>
				<input type="submit" value="Submit"/>
			</td>
		</tr><tr>
			<td></td>
			<td colspan=3>
				<table border=0 cellpadding=0 cellspacing=0 width="100%">
					<tr>
						<td class="rules">
							<LI>Supported file types are: GIF, JPG, PNG 
							<LI>Maximum file size allowed is <?= ini_get("upload_max_filesize") ?>. 
							<LI>Images greater than 100x100 pixels will be thumbnailed. 
							<LI>Read the <a href="/offensive/?c=faq">rules</a> before posting. 
							<LI><img src="jpn-flag.jpg"> <a href="http://en.wikipedia.org/wiki/Knowledge">チンポについて</a> - <a href="http://en.wikipedia.org/wiki/Penis">翻訳</a>
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
</form>
</div><hr>

<?php

  $page_limit_clause = $p * 15;
    
$sql = "SELECT
  offensive_uploads.id,
  offensive_uploads.timestamp,
  offensive_uploads.filename,
  offensive_uploads.nsfw,
  offensive_uploads.tmbo,
  offensive_uploads.type,
  offensive_count_cache.good as goods,
  offensive_count_cache.bad as bads,
  offensive_count_cache.tmbo AS tmbos,
  offensive_count_cache.comments,
  users.username as user_username,
  users.userid as user_userid
FROM
  offensive_uploads USE KEY (t_t_id)
LEFT JOIN offensive_count_cache
ON  offensive_count_cache.threadid = offensive_uploads.id
JOIN users
ON offensive_uploads.userid = users.userid
WHERE
  offensive_uploads.type= 'image'
AND
  offensive_uploads.status = 'normal'
AND
  users.account_status != 'locked'
ORDER BY offensive_uploads.timestamp DESC, offensive_uploads.id DESC
LIMIT $page_limit_clause, 15";

$result = tmbo_query($sql);
while( $image = mysql_fetch_assoc( $result ) ) 
{
	$upload = new Upload($image);
	$filepath = $upload->file();
?>


<!-- loopy -->
<span class="filesize">
	File: <a href="<?= "/offensive/pages/pic.php?id=".$upload->id() ?>"><?= htmlEscape($upload->filename()) ?></a> - (<?= byte_format(filesize($filepath))?>, <?

$info = getimagesize($filepath);
echo $info[0]."x".$info[1];

?>)
</span>
<br>




<a href="<?= $upload->URL() ?>"
	<? if($upload->filtered()) { ?>
		onMouseOver='changesrc("th<?= $upload->id()?>","<?= $upload->thumbURL() ?>")'
 		onMouseOut='changesrc("th<?= $upload->id() ?>","/offensive/graphics/th-filtered.gif")'
	<? } ?>
><img name="th<?= $upload->id()?>"
	src="<?= $upload->filtered()
		? "/offensive/graphics/th-filtered.gif" 
		: $upload->thumbURL() ?>"
 	border=0 align=left hspace=20 title="<?= byte_format(filesize($filepath)); ?>" /></a>
<?

?><a name="im<?= $upload->id() ?>"></a>
<!-- voting goes here -->
<span class="filetitle"></span> 
<span class="postername"><?= htmlEscape($upload->uploader()->username()) ?></span>&nbsp;
<?= date("m/d/y(D)H:i:s", $time) ?>
<span></span>
<span id="nothread<?= $upload->id() ?>">
	<a href="/offensive/pages/pic.php?id=<?= $upload->id() ?>" class="quotejs">No.<?= $upload->id() ?></a>
	&nbsp; [<a href="/offensive/?c=comments&fileid=<?= $upload->id() ?>">Reply</a>]
</span><br />
<!--<blockquote>So ur with ur honey and yur making out wen the phone rigns. U anser it n the vioce is &quot;wut r u doing wit my daughter?&quot; U tell ur girl n she say &quot;my dad is ded&quot;. THEN WHO WAS PHONE?
<br /><span class="abbr">Comment too long. Click <a href="res/60550329.html#60550329">here</a> to view the full text.</span></blockquote>-->

<!-- if(replies) -->

<?
	$sql = "SELECT offensive_comments.*, offensive_comments.id as commentid, offensive_comments.timestamp AS comment_timestamp, users.*
				FROM offensive_uploads, offensive_comments, users
				WHERE users.userid = offensive_comments.userid
				AND offensive_uploads.id=fileid AND fileid = " . $upload->id() . "
				AND comment != ''
				ORDER BY comment_timestamp";
	
	$res = tmbo_query( $sql );
	$rows = mysql_num_rows($res);
	$op = 0;
	$fetch = 0;

	$comment = mysql_fetch_assoc($res);
	++$fetch;
	if($comment['userid'] == $upload->uploader()->id()) {
		$op = 1;
		poast($comment, $image, $upload);
	}
	 

	if($rows - $op > 3) {
?>
<span class="omittedposts"><?= ($rows - 3 - $op) ?> post<?= ($rows - 3 - $op) == 1 ? "" : "s" ?> omitted. Click Reply to view.</span>
<?
	}

	if($op == 1) {
		if(!($comment = mysql_fetch_assoc($res))) {
			$comment = false;
		}
		++$fetch;
	}

	while($fetch <= $rows - 3) {
		$comment = mysql_fetch_assoc($res);
		++$fetch;
	}

	do {
		if(!$comment) break;
?>

<a name="<?= $comment['commentid'] ?>"></a>
<table>
	<tr>
		<td nowrap class="doubledash">&gt;&gt;</td>
		<td id="<?= $comment['commentid'] ?>" class="reply">
			<span class="replytitle"></span> 
			<span class="commentpostername"><?= $comment['username'] ?></span>&nbsp;<?=
			date("m/d/y(D)H:i:s", strtotime($comment['comment_timestamp']));
			?><span></span>
			<span id="norep<?= $comment['commentid'] ?>">
				<a href="/offensive/?c=comments&fileid=<?= $upload->id() ?>#<?= $comment['commentid'] ?>" class="quotejs">No.<?= $comment['commentid'] ?></a>
			</span>
			<? poast($comment, $image, $upload); ?>
		</td>
	</tr>
</table>

<?
	++$fetch;
	} while($comment = mysql_fetch_assoc($res));
?>

<!-- /if -->
<br clear=left><hr>
<!-- /loopy -->

<?php
}
?>

<table align=right>
	<tr>
		<td align="right">
			Style [<a href="#" onclick="setActiveStyleSheet('Yotsuba'); return false;">Yotsuba</a> | 
			<a href="#" onclick="setActiveStyleSheet('Yotsuba B'); return false;">Yotsuba B</a> | 
			<a href="#" onclick="setActiveStyleSheet('Futaba'); return false;">Futaba</a> | 
			<a href="#" onclick="setActiveStyleSheet('Burichan'); return false;">Burichan</a>]
		</td>
	</tr>
</table>
<table class=pages align=left border=1>
	<tr>
		<td>Previous</td>
		<td>
<?
for($i = 0; $i < 10; $i++) {
	if($p == $i) {
		echo "[<b>".($i + 1)."</b>]";
	} else {
		echo "[<a href=\"/tmbo/?p=$i\">".($i + 1)."</a>]";
	}
}
?>
		</td>
		<form action="/tmbo/" method=get>
		<td>
			<input type=hidden name=p value="<?= ($p + 1) ?>">
			<input type=submit value="Next" accesskey="x">
		</td>
		</form>
	</tr>
</table>
<br clear=all>
<div id="footer">
<span id="navbot">

[
<a href="/offensive/?c=user&userid=499" title="asshat">a</a> /
<a href="/offensive/?c=discussions" title="Message Board">d</a> /
<a href="/offensive/?c=hof" title="Hall of Fame">h</a> /
<a href="/offensive/?c=referral" title="Invite a Friend">i</a>
<a href="/offensive/map/gmap.php" title="World Map">m</a> /
<a href="/offensive/?c=faq" title="Rules & Stuff">r</a> /
<a href="/offensive/?c=search" title="Search">s</a> /
<a href="/offensive/?c=yearbook" title="Yearbook">y</a> 
]

[<a href="/offensive/?c=changeblog" title="tmbo Status">status</a>]

</span>

<span id="navbotr">[<a href="/offensive/?c=discussions" target="_top">Home</a>]</span>

<br><br>
<center>
	<font size="2">
		- <a href="http://images.google.com/" target="_top">futaba</a> + 
		<a href="/offensive/?c=user&userid=143" target="_top">futallaby</a> + 
		<a href="/offensive/?c=user&userid=151" target="_top">yotsuba</a> -
		<br>
		All trademarks and copyrights on this page are owned by their respective parties. 
		Images uploaded are the responsibility of the Poster. Comments are owned by the Poster.
	</font>
</center>
</div>
</body></html>
