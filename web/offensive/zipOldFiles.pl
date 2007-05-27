#!/usr/bin/perl

# moves files older than x days from srcDir to destDir

use Archive::Zip;

if( $#ARGV lt 2 ) {
	print "zipOldFiles.pl\nZips files last modified x days ago.\n";
	print "Usage: zipOldFiles daysOld srcDir outputDir\n";	
	print "Example: ./zipOldFiles.pl 1 ./images/ zips\n";
	exit();
} else {
	($daysOld, $srcDir, $zipDir ) = @ARGV;
}

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked.
@pathToScript = $0 =~ /.*\//gi;
chdir $pathToScript[0];

opendir( DIRHANDLE, $srcDir ) or die( "Couldn't read src directory." );

my $secondsPerDay = 86400; # 60 * 60 * 24

# figure out the date of the files we're after
my ($yesterday,$yestermonth,$yesteryear) = (localtime( time() - ($daysOld * $secondsPerDay) ))[3,4,5];

my $seekDate = "$yestermonth" . "_" . "$yesterday" . "_" . "$yesteryear";

my $zip = Archive::Zip->new();

$yestermonth++; # month is zero based. increment it so it makes sense to humans.

# append leading zero when appropriate
if( $yestermonth < 10 ) {
	$yestermonth = "0" . $yestermonth;
}

if( $yesterday < 10 ) {
	$yesterday = "0" . $yesterday;
}

my $baseFilename = ($yesteryear + 1900) . "_" . $yestermonth . "_" . $yesterday;

open( MANIFEST, "> $zipDir/$baseFilename" . "_MANIFEST.txt" );

while( defined( $filename = readdir( DIRHANDLE ) ) ) {

	if( $filename ne "." && $filename ne ".." ) {
		
		$mod = ( stat( "$srcDir/$filename" ) )[9];
		
		my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime( $mod );
		
		$fileDate = "$mon" . "_" . "$mday" . "_" . "$year";
		
		if( $fileDate eq $seekDate ) {
		#	print "$fileDate --- Zipping $filename\n";
			$zip->addFile( "$srcDir/$filename" );
			print MANIFEST $filename . "\n";
		} else {
		#	print "skipping $filename\n";
		}
		
	}
}

close MANIFEST;

closedir( DIRHANDLE );

my $zipFile = $baseFilename . ".zip";

my $status = $zip->writeToFileNamed( "$zipDir/$zipFile" );

# print $status . "\n\n";
