#!/usr/bin/perl -w

use Image::Size;

($x, $y) = imgsize("images/thumbs/th-photoshop-3.0.jpg");

print "$x, $y\n";
