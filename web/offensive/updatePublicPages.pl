#!/usr/bin/perl

# runtime-detection of missing perl modules
my (@missing_modules);
BEGIN {
        eval qq{use DBI; }; push @missing_modules,"DBI" if ($@);
        eval qq{use File::Copy; }; push @missing_modules,"File::Copy" if ($@);
}

die "There are missing required modules: ",join(", ",@missing_modules) if (@missing_modules);

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

$sql = "SELECT up.id, up.filename 
		FROM offensive_uploads up, hall_of_fame hof
		WHERE hof.fileid = up.id
			AND hof.type = 'today'
			ORDER BY up.timestamp DESC";

$statement = $dbh->prepare( $sql );
$statement->execute();

my @files = ();

while( (my( $id, $filename ) = $statement->fetchrow_array()) ) {
#	print "$filename\n";
	push( @files, $filename );
}

$dir = createTodayDirectory();

open( PUBLIC_FILE, ">public.php" ) or die("couldn't create public.php file.\n");
print PUBLIC_FILE "<?\n \$images = array();";
for( $i = 0; $i < $#files + 1; $i++ ) {
	$filename = $files[$i];
	$filename =~ s/\"/\\\"/g; # escape quotes
	print PUBLIC_FILE qq ^\n \$^ . qq ^images\[$i] = "$filename"\;^;
	if( -e "images/picpile/$files[$i]" ) {
		copy( "images/picpile/$files[$i]", "$dir/images/$files[$i]" );
	}
	if( -e "images/thumbs/th-$files[$i]" ) {
		copy( "images/thumbs/th-$files[$i]", "$dir/thumbs/th-$files[$i]" );
	}
}
print PUBLIC_FILE "\n?>";
close( PUBLIC_FILE );

copy( "public.php", "$dir/public.php" );
copy( "dailyIndex.php", "$dir/index.php" );
copyAssets( "$dir/assets/" );

open( LATEST_DAILY, ">daily/index.php" ) or die("couldn't create daily.php file.\n");
print LATEST_DAILY qq ^<? header("Location: ../$dir") ?>^;
close( LATEST_DAILY );

sub copyAssets {
	$dest = @_[0];
	
	copy( "../styles/oldskool.css", $dest );
	copy( "pages/styles.php", $dest );	
	copy( "graphics/loadingThumb.png", $dest );
	copy( "pages/offensive.js", $dest );
	copy( "graphics/previewNotAvailable.gif", $dest );
	copy( "graphics/dailydigest.png", $dest );	
	
}

sub zeroPad {
	$input = @_[0];

	if( $input < 10 ) {
		return "0" . $input;
	}
	
	return $input;
}

sub createTodayDirectory {

	my( $sec, $min, $hour, $day, $month, $year ) = localtime();
	
	$year += 1900;
	$month++;
	
	$month = zeroPad( $month );
	$day = zeroPad( $day );	
	
	if( ! -e "$year" ) {
		mkdir( "$year", 0755 ) or die "couldn't create year dir '$year'";
	}
	
	if( ! -e "$year/$month" ) {
		mkdir( "$year/$month", 0755 ) or die "couldn't create month dir '$month'";
	}

	if( ! -e "$year/$month/$day" ) {
		mkdir( "$year/$month/$day", 0755 ) or die "couldn't create day dir '$day'";
	}
	
	mkdir( "$year/$month/$day/images", 0755 );
	mkdir( "$year/$month/$day/thumbs", 0755 );
	mkdir( "$year/$month/$day/assets", 0755 );
	
	return "$year/$month/$day/";

}
