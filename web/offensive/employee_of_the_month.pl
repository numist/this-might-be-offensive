#!/usr/bin/perl

use lib qw(../.perl/lib);
use Mysql;
use DBI;

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


$host = "66.228.121.115";							#<-- Set host name
$database = "thismig_themaxx";									#<-- Set database name

my $dsn = 'DBI:mysql:themaxx:mysql.themaxx.com';
my $db_user_name = 'db_themaxx';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}


my $sql = "SELECT count( vote ) AS thecount, username, users.userid
			FROM offensive_comments, offensive_uploads, users
			WHERE vote = 'this is good'
				AND fileid = offensive_uploads.id
				AND offensive_uploads.userid = users.userid
				AND users.username != 'Fipi Lele'
				AND offensive_uploads.timestamp > DATE_SUB( now( ) , INTERVAL 1 MONTH )
			GROUP BY offensive_uploads.userid
			ORDER BY thecount DESC
			LIMIT 1";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();

	my( $count, $username, $userid ) = $statement->fetchrow_array();
	
	open( EOM_FILE, ">employeeOfTheMonth.txt" ) or die("couldn't create employee of the month file.\n");
	
	print EOM_FILE qq ^<div class="orange">employee of the month: <a class="orange" href="./?c=user&userid=$userid">$username</a> ($count good votes)</div>\n^;

	close( EOM_FILE );
