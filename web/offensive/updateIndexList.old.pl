#!/usr/bin/perl

use Mysql;
use DBI;
use Image::Size;

# don't execute from the web
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked. (the primary reason for this is that
# we intend to run it periodically via crontab.) $0 gives us the path
# to the script.

# grab everything up to the last slash.
@pathToScript = $0 =~ /.*\//gi;

# change to that directory.
chdir $pathToScript[0];


$host = "mysql.themaxx.com";							#<-- Set host name
$database = "db_themaxx";									#<-- Set database name

my $dsn = 'DBI:mysql:themaxx:mysql.themaxx.com';
my $db_user_name = 'db_themaxx';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}


#my sql = "SELECT count( c.id ) , c.vote, up .id, up.filename 
#FROM offensive_comments c, offensive_uploads up
#WHERE c.fileid = up.id
#AND vote
#GROUP  BY fileid, vote
#ORDER  BY up.timestamp DESC 
#LIMIT 100";

my $sql = "SELECT up. * , users.username, comments.vote, count( comments.id ) 
			FROM offensive_uploads up, users
			LEFT  JOIN offensive_comments comments ON up.id = comments.fileid
			WHERE up.userid = users.userid
			AND type='image'
			AND users.account_status != 'locked'
			GROUP  BY up.id, vote
			ORDER  BY up.timestamp DESC 
			LIMIT 300";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();

	$THUMBS_PER_ROW = 4;
	
	open( LIST_FILE, ">indexList.txt" ) or die("couldn't create indexList.txt file.\n");
	open( THUMB_FILE, ">indexListThumbnails.txt" ) or die("couldn't create indexListThumbnails.txt file.\n");
	print LIST_FILE qq ^<table width="100%">\n^;
	print THUMB_FILE qq ^<table width="100%" class="thumbnails">\n^;

	my $css = "evenfile";
	my $previousId = -1;
	my $good = 0;
	my $bad = 0;	
	my $comments = 0;
	my $output = 0;
	while( (my( $id, $userid, $filename, $timestamp, $ip, $nsfw, $tmbo, $type, $hash, $username, $vote, $comment_count ) = $statement->fetchrow_array()) && $output < 101) {

		if( $previousId > 0 && $previousId != $id ) {
		
			$css = ($css eq "odd_row") ? "even_row" : "odd_row";
			$nsfwMarker = $previousNsfw == 1 ? "[nsfw]" : "";
			my $truncatedFilename = substr( $nsfwMarker . " " . $previousFilename, 0, 60 );
			print LIST_FILE qq ^
			<tr class="$css">
				<td class="$css"><a href="pages/pic.php?id=$previousId" class="$css" title="uploaded by $previousUsername">$truncatedFilename</a></td>
				<td class="$css" style="text-align:right"><a href="./?c=comments&fileid=$previousId" class="$css">$comments comments</a> (+$good -$bad)</td>
			</tr>^;
#			<td class="$css"><a href="./?c=user&userid=$previousUserid">$previousUsername</a></td>

			if( $output % $THUMBS_PER_ROW == 0 ) {
				print THUMB_FILE "<tr>";
			}
			emitThumbnailRow( $previousId, $previousFilename, $comments, $good, $bad, $previousNsfw );
			if( $output % $THUMBS_PER_ROW == $THUMBS_PER_ROW - 1  ) {
				print THUMB_FILE "</tr>";
			}

			$good = 0;
			$bad = 0;
			$comments = 0;
			$output++;
		}

		if( $vote eq 'this is good' ) {
			$good = $comment_count;
		}
		elsif( $vote eq 'this is bad' ) {
			$bad = $comment_count;
		}
		
		$comments += $comment_count;
		$previousId = $id;
		$previousFilename = $filename;
		$previousUsername = $username;
		$previousUserid = $userid;
		$previousNsfw = $nsfw;

	}

	print LIST_FILE qq ^</table>\n^;
	close( LIST_FILE );
	
	if( $output % $THUMBS_PER_ROW != $THUMBS_PER_ROW - 1 ) {
		print THUMB_FILE "</tr>";
	}

	print THUMB_FILE qq ^</table>\n^;
	close( THUMB_FILE );	


sub emitThumbnailRow {
	my( $id, $filename, $comments, $good, $bad, $nsfw ) = @_;	
	my $css = $nsfw == 1 ? "nsfw" : "";

	my ($width, $height) = imgsize("images/thumbs/th-$filename");

	print THUMB_FILE qq^ 
	<td>
		<a href="pages/pic.php?id=$id"><span class="$css"><img src="images/thumbs/th-$filename" width="$width" height="$height" border="0"/></span></a><br/>
		<a href="?c=comments&fileid=$id">$comments comments (+$good -$bad)</a>
	</td>^;

}
