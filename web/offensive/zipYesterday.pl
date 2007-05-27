#!/usr/bin/perl -w

if( $#ARGV lt 0 ) {
	print "zipYesterday.pl\nZips yesterdays images.\n";
	print "Usage: zipYesterday.pl outputDir\n";	
	print "Example: ./zipYesterday zips\n";
	exit();
} else {
	($zipDir) = @ARGV;
}

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked.
@pathToScript = $0 =~ /.*\//gi;
chdir $pathToScript[0];

my $secondsPerDay = 86400; # 60 * 60 * 24

# figure out the date of the files we're after
my ($yesterday,$yestermonth,$yesteryear) = (localtime( time() - $secondsPerDay ))[3,4,5];


#my $seekDate = "$yestermonth" . "_" . "$yesterday" . "_" . "$yesteryear";

#my $zip = Archive::Zip->new();

$yestermonth++; # month is zero based. increment it so it makes sense to humans.

# append leading zero when appropriate
if( $yestermonth < 10 ) {
	$yestermonth = "0" . $yestermonth;
}

if( $yesterday < 10 ) {
	$yesterday = "0" . $yesterday;
}

$yesteryear += 1900;

my $sourceDir = "uploads/$yesteryear/$yestermonth/$yesterday/image";

my $baseFilename = $yesteryear . "_" . $yestermonth . "_" . $yesterday;

`zip -r $zipDir/$baseFilename $sourceDir`;

exit();
