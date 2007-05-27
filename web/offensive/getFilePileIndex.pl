#!/usr/bin/perl

#################################
# Original version copyright 2003 by Ray Hatfield. 
# http://themaxx.com/
#################################

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
my $db_user_name = 'db_themaxx';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}

$prepared_statement = $dbh->prepare("insert into offensive_uploads( userid, filename ) values(143, ?)");

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked. (the primary reason for this is that
# we intend to run it periodically via crontab.) $0 gives us the path
# to the script.

# grab everything up to the last slash.
@pathToScript = $0 =~ /.*\//gi;

# change to that directory.
chdir $pathToScript[0];

#################################
# global variables
#################################

# variable for filehandle to log file.
$LOGFILE;

# local paths (relative to script location) to save downloaded images to.
$saved_top_100_dir = "images/top100";
$saved_pics_pile_dir = "images/picpile";

# create a user agent to execute the http requests
$userAgent = LWP::UserAgent->new;
$userAgent->agent("themaxx.com/0.1");

# we need to store session cookies to remain logged in to filepile.
$cookies = HTTP::Cookies->new;

# urls we're going to retrieve for the file lists.
$top100Url = "http://www.filepile.org/archives/top100.php";
$picsPileUrl = "http://www.filepile.org/";

# how many files we already have and did not retrieve.
$skipCount = 0;

# username and password for logging into filepile.
$fpUsername = "filebot";
$fpPassword = "qazwsx";

# types of files to retrieve
@extensions = ("jpg","gif","png");

doIt();

$dbh->disconnect();

#################################
# doIt-- the main function that executes the update.
#################################
sub doIt {
	# anything we want to do before we start pulling files?
	beginUpdate();
	
	# hash of files we have. maps filename to modification time.
	%existing_files = ();
	
	# hash of filenames to filepile ids (used to link to comments on filepile)
	%file_ids = ();
	
	# initialize the exsisting_files hash 
##	getExistingFiles( $saved_top_100_dir );
	getExistingFiles( $saved_pics_pile_dir );
	
	logIn();
	
	getFilePile( $picsPileUrl, $saved_pics_pile_dir );
#	getFilePile( $top100Url, $saved_top_100_dir );
	
	logThis( "File retreival complete. Skipped " . $skipCount . " files." );
#	logThis( "Writing static files." );
#		
#	writeStaticList();
#	
#	logThis( "Update complete." );
	
	endUpdate();
}


#################################
# getExistingFiles-- loads existing_files hash with filenames from specified directory.
# takes one argument, the path to the directory containing previously saved files.
#################################
sub getExistingFiles {
	my $img_dir = $_[0];
	opendir( DIRHANDLE, $img_dir ) or die( "Couldn't read image directory." );
	while( defined( $filename = readdir( DIRHANDLE ) ) ) {
		if( $filename ne "." && $filename ne ".." && $filename ne ".DS_Store" ) {
			if( -e ($img_dir . "/" . $filename) ) {
				$stat = stat( $img_dir . "/" . $filename );
				$existing_files{ $img_dir . "/" . $filename } = $stat->mtime;
			}
		}
	}
	closedir( DIRHANDLE );
}

#################################
# getImages-- 
#################################
sub getImages {

	my ( $base_url, $img_dir, @urls ) = @_;
	
	# run in reverse order so the files are added oldest first.
	foreach my $url ( reverse @urls ) {
		
		# decode the url so we can extract the filename and see if we already have it.
		# filepile escapes the entire filename, including regular characters.
		my $decoded_url = unescape( $url );

		my $ext = join( "|", @extensions );
		
		my @fName = ($decoded_url =~ /\/([^\/]+\.(?:$ext))/i);
				
		if( $#fName eq -1 ) {
			next;
		}
				
		# re-escape the filename so html characters (spaces) are escaped.
		my $escaped = escape( $fName[0] );
		my @temp = ($fName[0] =~ /\&file=([^\&]+)/i );

		my $filename = $temp[0];

		if( $filename =~ /\[fpo[nly]*\]/i ) {
			next;
		}

		# get the file id so we can link to the comments on filepile.

		my $fileid = ($url =~ /\&id=([^\&]+)/i)[0];

#		my @url_parts = split( "/", $decoded_url );
#		if( $#url_parts gt 2 ) {
#		
#			# file id is between second to last set of slashes in the url.
#			my $fileid = $url_parts[$#url_parts];
#			if( $fileid eq "0" ) {
#				$fileid = $url_parts[$#url_parts - 1];
#			}			
		if( !($fileid eq "") ) {
			$file_ids{ $filename } = $fileid;
		} else {
			logThis( "Couldn't identify file id from url: $url" );
		}

		
		if( !needFile( $img_dir . "/" . $filename ) ) {
			
	#		logThis( "Already have '$fName[0]'.\n" );
			$skipCount++;
						
		} else {
			
			# get url, using top100 page as a referrer.
			my $response = getPage( "http://www.filepile.org/" . $url, $picsPileUrl );
			
			if( $response->is_success ) {
			
				my $img_src = getImageSrc( $response->content );
#				my $img_src = $base_url . $url;
								
				my @filename = ($img_src =~ /[^\?]+\/([^\?]+)/i );
	
				if( $#filename eq 0 ) {

					my $unescaped = unescape($filename);
					
					if( !needFile( $img_dir . "/" . $unescaped ) ) {
						logThis( "Skipping " . $unescaped . "\n" );
					} else {
						logThis( "Getting: " . $unescaped . "\n" );
#						my $imgResponse = getPage( $img_src );
						my $savePath = $img_dir;
						
						getUrlToFile( $img_src, "$img_dir/$unescaped" );
						
						if( -e $img_dir . "/" . $filename && ( (-s ($img_dir . "/" . $filename)) != 1680 ) ) {
							$stat = stat( "$img_dir/$unescaped" );						
							$existing_files{ "$img_dir/$unescaped" } = $stat->mtime;
							addToDatabase( "$unescaped" );
						}
					}
					
				} else {
	
					logThis( "multiple filenames?\n" );
					foreach( @filename ) {
						logThis( "\t$_\n" );
					}
	
				}
				
			}
		}
	}
}


#################################
# addToDatabase-- adds an entry to the database
#################################
sub addToDatabase {

	my $filename = $_[0];
	
#	$sql = "INSERT INTO offensive_uploads ( userid, filename  ) VALUES ( 143, '$filename' )";

	$prepared_statement->execute($filename);

#	$error_message = $dbh->errmsg;
#	if ($error_message) {
#		print $ssql."\n";
#		print $error_message."\n";
#	}
	
}


#################################
# login-- logs in to filepile and fills $cookies based on the response.
#################################
sub logIn {

	logThis( "logging in.\n" );

	my $req = HTTP::Request->new( POST => 'http://www.filepile.org/sign-in.php' );	

	$req->content_type('application/x-www-form-urlencoded');

#    $req->content('login_user=' . $fpUsername . '&login_password=' . $fpPassword . '&ret=%2F');

    $req->content('login_user=' . $fpUsername . '&login_password=' . $fpPassword . '&stay_logged=1&ret=%2F');

	my $response = $userAgent->request( $req );

	$cookies->extract_cookies( $response );
	
}

#################################
# needFile-- returns true or false indicating whether we should download the specified file.
# takes a single argument: the filename in question
#################################
sub needFile {
	my $filename = $_[0];
	
	$needIt = 1;
	if( $existing_files{ $filename } && ( (-s ($filename)) != 1680 )) {
		$needIt = 0;
	}
	
	return $needIt;
}

#################################
# getPage-- returns a response object representing the page at the specified url.
# takes a url agrument and optional referer:
# $response = getPage( 'http://whatever.org' [, $referer ] )
#################################
sub getPage {

	# create a request
	my $url = $_[0];
	my $req = HTTP::Request->new( GET => $url );
	
	$cookies->add_cookie_header( $req );
	
	# if a referer was provided, add the header accordingly.
	if( $#_ gt 0 ) {
		$req->headers->referer( $_[1] );
	}
	
	my $response = $userAgent->request( $req );
	
	unless( $response->is_success ) {
		logThis( "couldn't get url: " . $url . "\n" );
		logThis( $response->as_string . "\n" );
	}
	
	return $response;

}

#################################
# getUrlToFile-- retrieves a provided url and saves it to the provided filename.
#################################
sub getUrlToFile {

	# create a request
	my $url = $_[0];
	my $filename = $_[1];
	my $req = HTTP::Request->new( GET => $url );
	
	$cookies->add_cookie_header( $req );
	
	# if a referer was provided, add the header accordingly.
	if( $#_ gt 1 ) {
		$req->headers->referer( $_[2] );
	}
	
	my $response = $userAgent->request( $req, $filename );
	
	unless( $response->is_success ) {
		logThis( "couldn't get url: " . $url . "\n" );
		logThis( $response->as_string . "\n" );
	}
	
	return $response;
	
}

#################################
# extracts the main image src from a filepile image page.
# takes a single argument-- the src of the page
# $img_src = getImageSrc( "<html><head></head>...</html>" );
#################################
 sub getImageSrc {
 	
 	my $html_src = $_[0];
 	 	
 	# currently assumes it's the only img tag on the page
 	@src = $html_src =~ /<img[^>]+src="([^"]+)"/gi;
# 	logThis( "\n\nimg src:" . $#src . "\n" );

# 	my $i = 0;
#	foreach( @src ) {
# 		logThis( "$i: \t\t\t$_" );
# 		$i++;
# 	}
 	
#	logThis( @src[1] );
 	
 	return $src[1];
 	
 }

#################################
# returns the current local time as year/month/day hour:minute:second padded with
# leading zero when appropriate
#################################
sub getTimestamp {

	($sec,$min,$hour,$day,$mon,$year,$wday,$yday,$isdst) = localtime((time));
	
	# month is zero-based. we want 1 = january, not 0 = january.
	$mon++;

	# a little y2k compliance...
	if($year < 99) {
		$fullyear = $year + 2000;
	} else {
		$fullyear = $year + 1900;
	}
	
	# append leading zeros when appropriate
	$mon = zeroPad( $mon );
	$day = zeroPad( $day );	
	$hour = zeroPad( $hour );	
	$min = zeroPad( $min );	
	$sec = zeroPad( $sec );	
	
	return "$fullyear/$mon/$day $hour:$min:$sec";

}

#################################
# zeroPad-- pads numbers less than 10 with a leading zero.
# takes one argument, the original number.
#################################
sub zeroPad {
	my $num = $_[0];
	return $num < 10 ? '0' . $num : $num;
}

#################################
# logThis-- writes the passed in message to a log file in addition to printing it
# to the console.
#################################
sub logThis {
	print "\n" . getTimestamp() . " -- " . $_[0];
	print LOGFILE "\n" . getTimestamp() . " -- " . $_[0];
}

#################################
# beginUpdate-- opens log file and creates a temp file indicating
# that an update is in progress
#################################
sub beginUpdate {
	open( UPDATING_FILE, ">>updating.txt" );
	print UPDATING_FILE getTimestamp() . " -- updating\n";
	close( UPDATING_FILE );
	
	open( LOGFILE, ">>log.txt" ) or die("could not open log file.");
}


#################################
# endUpdate-- closes log file and deletes the temp file indicating that an update is
# in progress
#################################
sub endUpdate {
	if( -e "./updating.txt" ) {
		$num_deleted = unlink( "./updating.txt" );
	}
	logThis( "Deleting 'updating.txt' returned $num_deleted.\n" );
	close( LOGFILE );
}

#################################
# getFilePile-- 
#################################
sub getFilePile {

	my $list_url = $_[0];
	my $save_dir = $_[1];
	
	$response = getPage( $list_url );

	my $html = $response->content();
	
	# build a regular expression to match each file extension in @extensions
	my @regex = ();		
	foreach $ext (@extensions) {
		push( @regex, "[^\/]+\.$ext" );
	}
	my $exp = join( "|", @extensions );
	
	# extract the urls to the pages with images.
		
	@img_urls = ( $html =~ /href="([^"]+download.php\?type=pictures[^"]+)"[^>]+title="[^"]+\.(?:$exp)/gi );
#	@img_urls = ( $html =~ /href="([^"]+download\/pictures[^"]+)"[^>]+title="[^"]+\.(?:$exp)/gi );
	
	$num_imgs = $#img_urls + 1;
	logThis( $num_imgs . " jpgs on the list at '$list_url'.\n" );
	
	getImages( $list_url, $save_dir, @img_urls  );

}

#################################
# hashSortDescendingNum-- 
#################################
sub hashSortDescendingNum {
	$existing_files{$b} <=> $existing_files{$a}
}

