#!/usr/bin/perl

#################################
# Original version copyright 2003 by Ray Hatfield. This code may be freely modified
# and redistributed for non-commercial use as long as this notice remains in place.
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
#use Mysql;
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
#	getExistingFiles( $saved_top_100_dir );
#	getExistingFiles( $saved_pics_pile_dir );
	
#	logIn();
	
#	getFilePile( $picsPileUrl, $saved_pics_pile_dir );
#	getFilePile( $top100Url, $saved_top_100_dir );
	
#	logThis( "File retreival complete. Skipped " . $skipCount . " files." );
#	logThis( "Writing static files." );
		
#	writeStaticPages("picpile");
#	writeStaticPages("top100");

	writeStaticList();
	
	logThis( "Update complete." );
	
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
						
						if( -e $img_dir . "/" . $filename ) {
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

	my $req = HTTP::Request->new( POST => 'http://www.filepile.org/login.php' );	

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
#	print "\n" . getTimestamp() . " -- " . $_[0];
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
	close( $LOGFILE );
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


#################################
# writeStaticList-- writes the list that gets picked up by the index page.
#################################
sub writeStaticList {
	
	my $vote_count = $dbh->prepare("SELECT vote, count( vote ) AS votecount FROM offensive_comments WHERE fileid=? GROUP BY vote DESC");
	
	my $statement = $dbh->prepare("SELECT offensive_uploads.id, filename, users.username, count( offensive_comments.id )  AS comment_count FROM offensive_uploads, users LEFT  JOIN offensive_comments ON fileid = offensive_uploads.id WHERE users.userid = offensive_uploads.userid GROUP  BY offensive_uploads.timestamp DESC  LIMIT 1000");
	$statement->execute();
	
	my $css = "evenfile";
	
	$previousId = -1;
	$nextId = -1;
	$currentId = -1;
		
	$commentCount = 0;
	
#	open( LIST_FILE, ">indexList.txt" ) or logThis("couldn't create indexList.txt file.\n");
	
#	print LIST_FILE qq ^<table width="100%">\n^;
	
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
		

		if( $currentId gt -1 ) {
			writePage( $currentFilename, $currentUsername, $previousId, $currentId, $nextId, $nextFilename, $commentCount );
#			print LIST_FILE qq ^<tr><td class="$css"><a href="pages/picpile/$currentId.php" class="$css">$currentFilename</a></td><td style="text-align:right" class="$css"><a href="comments.php?fileid=$currentId" class="$css" target="comments">comments ($commentCount)</a></td></tr>\n^;
		}
		
		$css = ($css eq "evenfile") ? "oddfile" : "evenfile";
		
	}
	
	# we buffered one, so we still need to output the last file.
	writePage( $nextFilename, $nextUsername, $currentId, $nextId, ($nextId - 1), "", $nextCommentCount );
#	print LIST_FILE qq ^<tr><td class="$css"><a href="pages/picpile/$nextId.php" class="$css">$nextFilename</a></td><td style="text-align:right" class="$css"><a href="comments.php?fileid=$nextId" class="$css" target="comments">comments ($nextCommentCount)</a></td></tr>\n^;

#	print LIST_FILE qq ^</table>\n^;
	
#	close( LIST_FILE );
	
}


#################################
# writePage-- writes a static page.
#################################
sub writePage {

	my( $imagename, $username, $previous_id, $current_id, $next_id, $next_filename, $commentCount ) = @_;
	
	open( STATIC_FILE, ">pages/picpile/" . $current_id . ".php" ) or logThis("couldn't create file: $current_id.php\n");
	
	print STATIC_FILE <<RESUME;

	<html>
		<head>
			<META NAME=\"ROBOTS\" CONTENT=\"NOARCHIVE\"><title>themaxx.com : [ this might be offensive ]</title>
			<link rel="stylesheet" type="text/css" href="../styles.css"/>
			<script type="text/javascript">
				self.file_id = "$file_ids{ $current_filename }";
			</script>
			<script type="text/javascript" src="../offensive.js"></script>

		</head>
		<body onload="doOnloadStuff()">

		<div id="heading">
	

	
	<!-- paypal donation form -->
	<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
	<input type="hidden" name="cmd" value="_xclick">
	<input type="hidden" name="business" value="geek\@themaxx.com">
	<input type="hidden" name="amount" value="5.00">	
	<input type="hidden" name="item_name" value="bandwidth">
	<input type="hidden" name="no_note" value="1">
	<input type="hidden" name="currency_code" value="USD">
	<input type="hidden" name="tax" value="0">
	<input type="image" src="https://www.paypal.com/images/x-click-but21.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">

	&nbsp;&nbsp;

RESUME


if( $previous_id gt -1 ) {
		print STATIC_FILE "<a href=\"" . $previous_id . ".php\">previous</a>\n";
	} else {
		# include 'previous' even when no link is available to prevent the other links
		# from being in a different place.
		print STATIC_FILE "<a name=\"previous\" style=\"visibility:hidden\">previous</a>\n";
	}
	print STATIC_FILE " . <a href=\"http://themaxx.com/offensive/\">index</a> . ";
	if( $next_id gt -1 ) {

		$next_filename =~ s/\"/&quot;/g;
		print STATIC_FILE "<a href=\"" . $next_id . ".php\" title=\"" . $next_filename . "\">next</a>\n";

	}
	
	print STATIC_FILE " . <a style=\"margin-left:64px;\" href=\"/offensive/comments.php?fileid=$current_id\" target=\"comments\">comments ($commentCount)</a>";
	
	$current_escaped = $imagename;

	# replace question marks in the file name	
	$current_escaped =~ s/\?/%3F/;

	# and pound signs
	$current_escaped =~ s/\#/%23/;

	
	print STATIC_FILE "<br /><br />" . $imagename . " <br/><span style=\"color:#999999\">uploaded by $username</span><br/><br/><img src=\"http://images.themaxx.com/mirror.php/offensive/images/picpile/$current_escaped\"/>";

	print STATIC_FILE "</body></html>";	
	
	close( STATIC_FILE );

}




#################################
# writeStaticPages-- takes one argument indicating which "pile" to create the static
# pages for. this allows us to write all of the top100 files at once and then write all
# of the picpile files. if we did them simultaneously the 'next' and 'previous'
# links would get mixed up.
#################################
sub writeStaticPages {
	
	$subdir = $_[0];
	my $previous;
	my $previous2;
		
	foreach $key (sort hashSortDescendingNum( keys(%existing_files) ) ) {
		# logThis( $existing_files{ $key } . "\n" );
		
		# skip the top100 pile
		unless( $key =~ /$subdir/i ) {
			next;
		}
		
		# get the filename without the directory portion
		my @filenames = $key =~ /[^\/]+$/gi;
		
		# should only be one match.
		my $filename = $filenames[0];
		
		my $current = $key;

		my %page = (
			"next" => $current,
			"previous" => $previous2,
			"current" => $previous
		);
		
		writeStaticPage( %page );
				
		$previous2 = $previous;
		$previous = $current;

	}
	
	# we're buffering to get the previous/next links, so we need one more page for the last item.
	my %page = (
		"previous" => $previous2,
		"current" => $previous
	);
	
	writeStaticPage( %page );

}

#################################
# writes a static html page for displaying a gallery page.
# takes a hash containing paths to 'current', 'previous', and 'next'.
# the filename will be deduced from 'current', and links will be created
# for previous and next, if present in the hash.
#################################
sub writeStaticPage {

	my %files = @_;
	# logThis( $files{current} . "\n" );
	
	unless( defined $files{"current"} ) {
		return;
	}
	
	my $previous = $files{"previous"};
	my $current = $files{"current"};
	my $next = $files{"next"};

	my $current_filename = ($current =~ /[^\/]+$/gi)[0];
	my $next_filename = ($next =~ /[^\/]+$/gi)[0];
	my $previous_filename = ($previous =~ /[^\/]+$/gi)[0];	


	my $page_subdir = ($current =~ /\/([^\/]+)\/[^\/]+$/gi)[0];
	
	#logThis( "creating page: $current_filename\n");
	
	open( STATIC_FILE, ">pages/$page_subdir/" . $current_filename . ".php" ) or logThis("couldn't create file: $current_filename.php\n");


	
print STATIC_FILE <<RESUME;

	<html>
		<head>
			<META NAME=\"ROBOTS\" CONTENT=\"NOARCHIVE\"><title>themaxx.com : [ this might be offensive ]</title>
			<link rel="stylesheet" type="text/css" href="../styles.css"/>
			<script type="text/javascript">
				self.file_id = "$file_ids{ $current_filename }";
			</script>
			<script type="text/javascript" src="../offensive.js"></script>

		</head>
		<body onload="doOnloadStuff()">

		<div id="heading">
	

	
	<!-- paypal donation form -->
	<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
	<input type="hidden" name="cmd" value="_xclick">
	<input type="hidden" name="business" value="geek\@themaxx.com">
	<input type="hidden" name="amount" value="5.00">	
	<input type="hidden" name="item_name" value="bandwidth">
	<input type="hidden" name="no_note" value="1">
	<input type="hidden" name="currency_code" value="USD">
	<input type="hidden" name="tax" value="0">
	<input type="image" src="https://www.paypal.com/images/x-click-but21.gif" border="0" name="submit" alt="Make payments with PayPal - it's fast, free and secure!">

	&nbsp;&nbsp;

RESUME

	
	if( defined( $previous ) ) {
		$previous_filename = ($previous =~ /[^\/]+$/gi)[0];
		print STATIC_FILE "<a href=\"" . escape( $previous_filename ) . ".php\">previous</a>\n";
	} else {
		# include 'previous' even when no link is available to prevent the other links
		# from being in a different place.
		print STATIC_FILE "<a name=\"previous\" style=\"visibility:hidden\">previous</a>\n";
	}
	print STATIC_FILE " . <a href=\"http://themaxx.com/offensive/\">index</a> . ";
	if( defined( $next ) ) {
		print STATIC_FILE "<a href=\"" . escape( $next_filename ) . ".php\">next</a>\n";
	}

	$current_escaped = $current;

	# replace question marks in the file name	
	$current_escaped =~ s/\?/%3F/;

	# and pound signs
	$current_escaped =~ s/\#/%23/;

	if( $file_ids{ $current_filename } ) {
	
		print STATIC_FILE <<RESUME;
		
		<!-- $current_escaped -->
		
		<?php if( \$_COOKIE["logged_in_fipi"] == 'true') { ?>
			<span id="comments_link">
				<a href="http://www.filepile.org/file.php?id=$file_ids{ $current_filename }" target="fp_comments">comments (fp)</a>
			</span>
		<?php } ?>


RESUME
}

	print STATIC_FILE "</form></div>";
	
#	print STATIC_FILE "<br /><br />" . $current_filename . "<br/><br/><img src=\"../../$current_escaped\"/>";
	print STATIC_FILE "<br /><br />" . $current_filename . "<br/><br/><img src=\"http://images.themaxx.com/mirror.php/offensive/images/picpile/$current_escaped\"/>";

	print STATIC_FILE "</body></html>";

	close( STATIC_FILE );

}

