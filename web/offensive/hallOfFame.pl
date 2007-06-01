#!/usr/bin/perl


#use Mysql;
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

## remove existing db entries
my $sql = "TRUNCATE TABLE hall_of_fame";
my $statement = $dbh->prepare( $sql );
$statement->execute();


## insert current top 100 list
$sql = "INSERT INTO hall_of_fame (fileid, votes, type)
		SELECT threadid, good AS thecount, 'hof'
			FROM offensive_count_cache, offensive_uploads
		WHERE offensive_uploads.id = threadid
			AND offensive_uploads.type = 'image'
		ORDER  BY thecount DESC , offensive_uploads.timestamp
		LIMIT 100";

$statement = $dbh->prepare( $sql );
$statement->execute();

$sql = "INSERT INTO hall_of_fame (fileid, votes, type)
		SELECT threadid, good AS thecount, 'today' AS type
			FROM offensive_count_cache, offensive_uploads up
		WHERE up.id = threadid
			AND up.type = 'image'
			AND up.timestamp >  DATE_SUB( now(), INTERVAL 1 DAY )
		ORDER  BY thecount DESC , up.timestamp
		LIMIT 100";


# $sql = "INSERT INTO hall_of_fame ( fileid, votes, type )
#			SELECT up.id, count( oc.vote ) as thecount, 'today' AS type
#				FROM offensive_comments oc, offensive_uploads up
#				WHERE up.timestamp >  DATE_SUB( now(), INTERVAL 1 DAY )
#					AND oc.vote = 'this is good'
#					AND oc.fileid = up.id
#					AND up.type = 'image'						
#				GROUP BY up.id
#				ORDER by thecount DESC
#				LIMIT 100";

$statement = $dbh->prepare( $sql );
$statement->execute();

## get the list
$sql = "SELECT fileid, filename FROM hall_of_fame, offensive_uploads WHERE offensive_uploads.id = fileid";

$statement = $dbh->prepare( $sql );
$statement->execute();

while( my( $id, $filename) = $statement->fetchrow_array() ) {
#	print $id . ":" . $filename;
	if( -e "images/picpile/" . $filename ) {
		copy "images/picpile/" . $filename, "images/halloffame/" . $filename;
	}
}
