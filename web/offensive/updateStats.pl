#!/usr/bin/perl

use Mysql;
use DBI;
use File::Copy;

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

my $dsn = 'DBI:mysql:themaxx:mysql.themaxx.com';
my $db_user_name = 'db_themaxx';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}

truncateTable();
insertVoteStats( "this is good" );
insertVoteStats( "this is bad" );
employeeOfTheMonth();
uploaders();
votesReceived( "this is good", "good_received" );
votesReceived( "this is bad", "bad_received" );

# emitTable( "haters.txt", "haters (bad votes cast)", "this is bad" );
# emitTable( "lovers.txt", "lovers (good votes cast)", "this is good" );


sub truncateTable() {
	my $sql = "TRUNCATE TABLE vote_stats";
	my $statement = $dbh->prepare( $sql );
	$statement->execute();	
}

sub insertVoteStats {
	my ($vote) = @_;

	my $sql = "INSERT INTO vote_stats( value, userid, type ) 
				SELECT count( vote )  AS votes, userid, \"$vote\" AS type 
					FROM offensive_comments
					WHERE vote = \"$vote\"
					GROUP BY userid";
	my $statement = $dbh->prepare( $sql );
	$statement->execute();						
}

sub employeeOfTheMonth {

	my $sql = "INSERT INTO vote_stats( value, userid, type )
					SELECT count( vote ) AS thecount, offensive_uploads.userid, \"employee\" AS type
					FROM offensive_comments, offensive_uploads, users
					WHERE vote = 'this is good'
						AND fileid = offensive_uploads.id
						AND offensive_uploads.userid = users.userid
						AND users.username != 'Fipi Lele'
						AND offensive_uploads.timestamp > DATE_SUB( now( ) , INTERVAL 1 MONTH )
					GROUP BY offensive_uploads.userid
					ORDER BY thecount DESC
					LIMIT 50";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();						

}

sub uploaders {

	my $sql = "INSERT INTO vote_stats( value, userid, type )
			SELECT count( id )  AS thecount, userid, \"user_uploads\" AS type
				FROM offensive_uploads
				WHERE userid <> 143
				GROUP BY userid";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();						

}

sub votesReceived {

	my ($vote,$type) = @_;
	
	my $sql = "INSERT INTO vote_stats( value, userid, type )
				SELECT count( vote )  AS thecount, offensive_uploads.userid, \"$type\" AS type
					FROM offensive_comments, offensive_uploads
					WHERE vote = '$vote'
					AND offensive_uploads.id = offensive_comments.fileid
					GROUP  BY offensive_uploads.userid";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();

}

sub emitTable {

	my( $filename, $heading, $vote ) = @_;

	my $sql = "SELECT count( vote )  AS votes, users.username, users.userid
		FROM  offensive_comments, users
		WHERE vote = '$vote' AND users.userid = offensive_comments.userid
		GROUP  BY users.userid
		ORDER  BY votes DESC 
		LIMIT 25";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();

	open( THE_FILE, ">$filename" ) or die("couldn't create file.\n");
	
	print THE_FILE qq ^
	<table width="100%" cellspacing="0" cellpadding="4">
		<tr><td colspan="3" class="heading">$heading</td></tr>
	^;

	my $count = 0;
	my $style = "even_row";
	while( my($votes, $username, $userid) = $statement->fetchrow_array() ) {
		$count++;
		$style = $style eq "odd_row" ? "even_row" : "odd_row";
		print THE_FILE qq ^
		<tr class="$style">
			<td style="text-align:right">$count</td>
			<td><a href="./?c=user&userid=$userid">$username</a></td>
			<td style="text-align:right">$votes</td>
		</tr>^;
	}

	print THE_FILE "</table>";

	close( THE_FILE );

}
