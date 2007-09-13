#!/usr/bin/perl

use lib qw(../.perl/lib);

# don't run if invoked from a browser.
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

# deletes files older than x days from srcDir to destDir

if( $#ARGV lt 1 ) {
	print "deleteOldFiles.pl\nDeletes files older than x days from srcDir\n";
	print "Usage: deleteOldFiles daysOld srcDir\n\n";	
	exit();
} else {
	($daysOld, $srcDir ) = @ARGV;
}

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked.
@pathToScript = $0 =~ /.*\//gi;
chdir $pathToScript[0];

opendir( DIRHANDLE, $srcDir ) or die( "Couldn't read src directory." );

while( defined( $filename = readdir( DIRHANDLE ) ) ) {
	if( $filename ne "." && $filename ne ".." ) {
		
		if( -M "$srcDir/$filename" > $daysOld ) {
			print "Deleting $filename\n";		
			# is it a problem to delete files while reading the directory?
			unlink( "$srcDir/$filename" );
		}
		
	}
}
closedir( DIRHANDLE );
