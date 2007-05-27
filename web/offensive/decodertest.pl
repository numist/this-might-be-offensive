#!/usr/bin/perl -w

use CGI qw/escape unescape/;

my $testString = 'P><a HREF="http://www.filepile.org/function.php/download/pictures/%68%6F%6D%65%6D%61%64%65%2D%73%69%6D%70%73%6F%6E%73%2D%66%69%67%75%72%69%6E%65%73%2E%6A%70%67/137446';

my $decoded = unescape( $testString );

print "$decoded";
