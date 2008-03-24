#!/usr/bin/perl

#use Mysql;
use DBI;
use ConfigReader::Simple;

# don't execute from the web
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

# Grab the configuration options, and then set some variables to use
# throughout the script.
my $config = ConfigReader::Simple->new("../admin/.config");
my $database_host = $config->get("database_host");
my $db_user_name = $config->get("database_user");
my $db_password = $config->get("database_pass");
my $database_name = $config->get("database_name");

# Connect to the database
my $dsn = "DBI:mysql:".$database_name.":".$database_host;
my $dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}


my $sql = "SELECT count( offensive_comments.vote ) AS thecount, users.username, users.userid 
   FROM offensive_comments
   JOIN offensive_uploads ON offensive_comments.fileid = offensive_uploads.id
   JOIN users ON offensive_uploads.userid = users.userid
   WHERE offensive_comments.timestamp > DATE_SUB( now( ) , INTERVAL 1 MONTH ) 
     AND offensive_comments.vote = 'this is good' 
   GROUP BY offensive_uploads.userid 
   ORDER BY thecount DESC LIMIT 1";

	my $statement = $dbh->prepare( $sql );
	$statement->execute();

	my( $count, $username, $userid ) = $statement->fetchrow_array();
	
	open( EOM_FILE, ">employeeOfTheMonth.txt" ) or die("couldn't create employee of the month file.\n");
	
	print EOM_FILE qq ^<div class="orange">employee of the month: <a class="orange" href="./?c=user&userid=$userid">$username</a> (+$count)</div>\n^;

	close( EOM_FILE );
