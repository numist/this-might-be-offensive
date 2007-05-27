#!/usr/bin/perl -w

use Mysql;
use DBI;
use Image::Magick;

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

my $db_update = (stat("./last_update.txt"))[9];
my $img_update = (stat("./comp.jpg"))[9];

if( $db_update lt $img_update ) {
	exit;
}

my $dsn = 'DBI:mysql:themaxx:mysql.themaxx.com';
my $db_user_name = 'db_themaxx';
my $db_password = 'db_password_goes_here';
$dbh = DBI->connect($dsn, $db_user_name, $db_password);

if (! $dbh) {										#<-- Make sure we got a valid connection
	print "No database handle\n";
	exit(0);
}

$image = Image::Magick->new;
$marker = Image::Magick->new;
$x = $image->Read( "maxxer_map_large.png" );
$x = $marker->Read( "marker.gif" );
my( $marker_width, $marker_height ) = $marker->Get('width','height');

my $sql = "SELECT users.username, x, y FROM users, maxxer_locations WHERE users.userid=maxxer_locations.userid";
my $statement = $dbh->prepare( $sql );
$statement->execute();
my $fontsize=14;
my $yoffset=$fontsize/2;
my $xoffset=$marker_width/2;
while( my( $username, $x, $y ) = $statement->fetchrow_array() ) {
	$image->Composite( image=>$marker,x=>$x-($marker_width/2),y=>$y-($marker_height/2) );
	$x += $xoffset;
	$y += $yoffset;
	$image->Annotate( x=>$x, y=>$y,pointsize=>14,fill=>'#000033',stroke=>'#333366',strokewidth=>4,text=>$username );
	$image->Annotate( x=>$x, y=>$y,pointsize=>14,fill=>'#ccccff',text=>$username );
}

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);

$year += 1900;
$mon += 1;
$mon = zeroPad( $mon );
$mday = zeroPad( $mday );
$hour = zeroPad( $hour );
$min = zeroPad( $min );
$sec = zeroPad( $sec );

$timestamp = "Last updated: $year-$mon-$mday $hour:$min:$sec PST";
$image->Annotate( x=>100, y=>100,pointsize=>14,fill=>'#ccccff',text=>$timestamp );

$image->Write('comp.jpg');

#################################
# zeroPad-- pads numbers less than 10 with a leading zero.
# takes one argument, the original number.
#################################
sub zeroPad {
	my $num = $_[0];
	return $num < 10 ? '0' . $num : $num;
}
