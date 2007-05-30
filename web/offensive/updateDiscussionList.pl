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

#my $sql = "SELECT up. * , users.username, comments.vote, count( comments.id ) 
#			FROM offensive_uploads up, users
#			LEFT  JOIN offensive_comments comments ON up.id = comments.fileid
#			WHERE up.userid = users.userid
#			AND type='image'
#			AND users.account_status != 'locked'
#			GROUP  BY up.id, vote
#			ORDER  BY up.timestamp DESC 
#			LIMIT 300";

my $sql = "CREATE temporary TABLE recent_discussions (
			id int(11) NOT NULL,
			userid int(11) NOT NULL default '0',
			filename varchar(255) NOT NULL default '',
			timestamp timestamp(19) NOT NULL,
			type enum('image','topic') NOT NULL default 'topic',
		PRIMARY KEY  (id),
		KEY timestamp (timestamp),
		KEY filename (filename),
		KEY userid (userid),
		KEY type (type)
);";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();


$sql = "insert into recent_discussions( id, filename, userid, timestamp, type )
			select id, filename, userid, timestamp, type
				from offensive_uploads where type='topic' AND status='normal'
			order by timestamp desc limit 100;";

	$statement = $dbh->prepare( $sql );
	$statement->execute();


#$sql = "SELECT up.id, up.userid, up.filename, up.timestamp, up.nsfw, up.tmbo, 
#		users.username, comments.vote, count( comments.id ) 
#			FROM recent_uploads up, users
#			LEFT JOIN offensive_comments comments ON up.id = comments.fileid
#			WHERE up.userid = users.userid
#			AND type='image'
#			AND users.account_status != 'locked'
#			GROUP  BY up.id, vote
#			ORDER  BY up.timestamp DESC";

$sql = "SELECT up.id, up.userid, up.filename, up.timestamp, 
		users.username, counts.comments
			FROM recent_discussions up, users
			LEFT JOIN offensive_count_cache counts ON up.id = counts.threadid
			WHERE up.userid = users.userid
			AND type='topic'
			AND users.account_status != 'locked'
			ORDER BY up.timestamp DESC";

	$statement = $dbh->prepare( $sql );
	$statement->execute();

	$THUMBS_PER_ROW = 4;
	
	open( LIST_FILE, ">discussionsList.txt" ) or die("couldn't create discussionList.txt file.\n");
	print LIST_FILE qq ^<table width="100%">\n^;

	my $css = "evenfile";
	my $output = 0;
	while( (my( $id, $userid, $filename, $timestamp, $username, $comments ) = $statement->fetchrow_array()) && $output < 101) {

		$css = ($css eq "odd_row") ? "even_row" : "odd_row";
		my $newFilename = substr( $filename, 0, 80);
		$comments = $comments == null ? 0 : $comments;
		
		print LIST_FILE qq ^
		<tr class="$css">
			<td class="$css"><div class="clipper"><a href="./?c=comments&fileid=$id" class="$css" title="started by $username">$newFilename</a></div></td>
			<td class="$css" style="text-align:right;white-space:nowrap"><a href="./?c=comments&fileid=$id" class="$css">$comments comments</a></td>
		</tr>^;

		$output++;


	}

	print LIST_FILE qq ^</table>\n^;
	close( LIST_FILE );
	
