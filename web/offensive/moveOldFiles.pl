#!/usr/bin/perl

# don't run if invoked from a browser.
if( $ENV{'DOCUMENT_ROOT'} ){
	print "Content-type:text/plain\n\nGo away.";
	exit();
}

# moves files older than x days from srcDir to destDir

if( $#ARGV lt 2 ) {
	print "moveOldFiles.pl\nMoves files older than x days from srcDir to destDir\n";
	print "Usage: moveOldFiles daysOld srcDir destDir\n\n";	
	exit();
} else {
	($daysOld, $srcDir, $destDir ) = @ARGV;
}

# set the current directory to the directory containing the script
# so our relative path references (images, etc) work regardless of
# where the script was invoked.
@pathToScript = $0 =~ /.*\//gi;
chdir $pathToScript[0];

opendir( DIRHANDLE, $srcDir ) or die( "Couldn't read src directory." );

while( defined( $filename = readdir( DIRHANDLE ) ) ) {
	if( $filename ne "." && $filename ne ".." ) {
		
		if( -M "$srcDir/$filename" ge $daysOld ) {
			print "Moving $filename\n";		
			# is it a problem to move files while reading the directory?
			rename( "$srcDir/$filename", "$destDir/$filename" );
		}
		
	}
}
closedir( DIRHANDLE );
