#!/usr/bin/perl

use strict;
use warnings;


# runtime-detection of missing perl modules
my (@missing_modules);
BEGIN {
    eval qq{ use DBI; };
	push @missing_modules,"DBI" if ($@);

    eval qq{ use Image::Size; };
	push @missing_modules,"Image::Size" if ($@);

    eval qq{ use ConfigReader::Simple; };
	push @missing_modules,"ConfigReader::Simple" if ($@);

    eval qq{ use CGI qw(escape escapeHTML); };
	push @missing_modules,"CGI" if ($@);
}

die "There are missing required modules: ",join(", ",@missing_modules) if (@missing_modules);

# don't execute from the web
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

# Tunable paramaters here:
# ===

my $THUMBS_PER_ROW = 4;

# ===


# grab everything up to the last slash.
my @pathToScript = $0 =~ /.*\//gi;

# change to that directory.
chdir $pathToScript[0];

# Grab the configuration options, and then set some variables to use
# throughout the script.
my $config = ConfigReader::Simple->new("../admin/.config", [qw(database_host database_user database_pass database_name)]);
my $database_host = $config->get("database_host");
my $database_user = $config->get("database_user");
my $database_pass = $config->get("database_pass");
my $database_name = $config->get("database_name");

# Connect to the database
my $dsn = "DBI:mysql:".$database_name.":".$database_host;
my $dbh = DBI->connect($dsn, $database_user, $database_pass);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}

my $recent_uploads_SQL = "CREATE temporary TABLE recent_uploads (
			id int(11) NOT NULL,
			userid int(11) NOT NULL default '0',
			filename varchar(255) NOT NULL default '',
			timestamp timestamp(19) NOT NULL,
			nsfw tinyint(4) default NULL,
			tmbo tinyint(4) NOT NULL default '0',
			type enum('image','topic') NOT NULL default 'image',
		PRIMARY KEY  (id),
		KEY timestamp (timestamp),
		KEY filename (filename),
		KEY userid (userid),
		KEY type (type)
);";
$dbh->do($recent_uploads_SQL);


my $populate_recent_uploads_SQL = "INSERT INTO recent_uploads( id, filename, userid, timestamp, nsfw, tmbo, type )
			SELECT id, filename, userid, timestamp, nsfw, tmbo, type
				FROM offensive_uploads WHERE type='image' AND status='normal'
			ORDER BY timestamp DESC LIMIT 100;";

my $populate_recent_uploads = $dbh->prepare( $populate_recent_uploads_SQL );
$populate_recent_uploads->execute();

my $uploads_sql = "SELECT up.id, up.userid, up.filename, up.timestamp, up.nsfw, up.tmbo, 
		users.username, counts.comments, counts.good, counts.bad
			FROM (recent_uploads up, users)
			LEFT JOIN offensive_count_cache counts ON (up.id = counts.threadid)
			WHERE up.userid = users.userid
			AND type='image'
			AND users.account_status != 'locked'
			ORDER BY up.timestamp DESC";

my $uploads_sth = $dbh->prepare( $uploads_sql );
$uploads_sth->execute();

	
open( LIST_FILE, ">indexList.txt" ) or die("couldn't create indexList.txt file ($!)");
open( THUMB_FILE, ">indexListThumbnails.txt" ) or die("couldn't create indexListThumbnails.txt file ($!)");
print LIST_FILE qq ^<table width="100%">\n^;
print THUMB_FILE qq ^<table width="100%" class="thumbnails">\n^;

my $css = "evenfile";
my $output = 0;
while( (my( $id, $userid, $filename, $timestamp, $nsfw, $tmbo, $username, $comments, $good, $bad ) = $uploads_sth->fetchrow_array()) && $output < 101) {

	$css = ($css eq "odd_row") ? "even_row" : "odd_row";
	my $nsfwMarker = $nsfw == 1 ? "[nsfw]" : "";
	my $newFilename = substr( $nsfwMarker . " " . $filename, 0, 80);
	$newFilename=escapeHTML($newFilename);

	# the database can contain NULL values for comments/good/bad
	# NULL values come back as undefined here in perl.
	$comments = defined($comments) ? $comments : 0;
	$good = defined($good) ? $good : 0;
	$bad = defined($bad) ?  $bad : 0;
		
	print LIST_FILE qq ^
\t\t\t	<tr class="$css">
\t\t\t		<td class="$css"><div class="clipper"><a href="pages/pic.php?id=$id" class="$css" title="uploaded by $username">$newFilename</a></div></td>
\t\t\t		<td class="$css" style="text-align:right;white-space:nowrap"><a href="./?c=comments&fileid=$id" class="$css">$comments comments</a> (+$good -$bad)</td>
\t\t\t	</tr>^;

	if( $output % $THUMBS_PER_ROW == 0 ) {
		print THUMB_FILE "<tr>";
	}

	emitThumbnailRow( $id, $filename, $comments, $good, $bad, $nsfw );

	if( $output % $THUMBS_PER_ROW == $THUMBS_PER_ROW - 1  ) {
		print THUMB_FILE "</tr>";
	}
	$output++;
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
	my $css = $nsfw == 1 ? ' class="nsfw"' : "";

	my ($width, $height) = imgsize("images/thumbs/th-$filename");

	# defaults, incase imagesize() returned nothing
	$width=$width||100;
	$height=$height||100;

	# this escapes for URI encoding (/ = %2F)
	$filename = escape($filename);

	print THUMB_FILE qq^ 
	<td>
\t	<a heef="pages/pic.php?id=$id"><span$css><img src="images/thumbs/th-$filename" width="$width" height="$height" border="0"/></span></a><br/>
\t	<a href="?c=comments&fileid=$id">$comments comments (+$good -$bad)</a>
	</td>^;

}
