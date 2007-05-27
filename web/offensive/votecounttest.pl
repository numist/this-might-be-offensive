#!/usr/bin/perl -w

# don't execute from the web
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

use LWP::Simple;
use LWP::UserAgent;
use HTTP::Cookies;
use File::stat;
use CGI qw/escape unescape/; #for decoding url-encoded strings
use Mysql;
use DBI;

$host = "mysql.themaxx.com";							#<-- Set host name
$database = "themaxx";									#<-- Set database name


my $dsn = 'DBI:mysql:themaxx:mysql.themaxx.com';
my $db_user_name = 'fleece';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}


writeStaticList();

#################################
# writeStaticList-- writes the list that gets picked up by the index page.
#################################
sub writeStaticList {
	
	my $vote_count = $dbh->prepare("SELECT vote, count( vote ) AS votecount FROM offensive_comments WHERE fileid=? GROUP BY vote");
	
	my $statement = $dbh->prepare("SELECT offensive_uploads.id, filename, users.username, count( offensive_comments.id )  AS comment_count FROM offensive_uploads, users LEFT  JOIN offensive_comments ON fileid = offensive_uploads.id WHERE users.userid = offensive_uploads.userid GROUP  BY offensive_uploads.timestamp DESC  LIMIT 100");

	$statement->execute();

	
	$previousId = -1;
	$nextId = -1;
	$currentId = -1;
		
	$commentCount = 0;
	
	
	while( my( $id, $filename, $username, $comment_count ) = $statement->fetchrow_array() ) {

		$previousId = $currentId;
		$currentId = $nextId;
		$nextId = $id;
		$currentFilename = $nextFilename;
		$nextFilename = $filename;

		$commentCount = $nextCommentCount;
		$nextCommentCount = $comment_count;

		$currentUsername = $nextUsername;
		$nextUsername = $username;


#### BAH!!! how do i get these values out!!?!?!?
		$vote_count->bind_param( 1, $id );
		$vote_count->execute();
		
		while( $votes = $vote_count->fetchrow_hashref ) {
			print $id . ": " . $votes->{'vote'} . " " . $votes->{ 'votecount' } . "\n";
		}
		
		print "\n\n";
	
		
	}
	
	
}
