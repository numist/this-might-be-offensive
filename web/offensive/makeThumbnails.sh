#!/bin/bash

## surely there's a way to hit all the extensions in one pass,
## but i don't know what it is.

## wooha, yehp. -jerry Sun Jan 30 12:49:03 CST 2005

PATH=/usr/local/bin:$PATH
export PATH

cd /hsphere/local/home/thismightbe/thismight.be/offensive

extset="jpg JPG jpeg JPEG gif GIF png PNG"

for ext in $extset ; do 
	for img in images/picpile/*.$ext ; do
		# when there are no matches, we get the odd result of echo saying the text
		# instead of returning a blank pattern match
		if [ "$img" != "images/picpile/*.$ext" ] ; then
			thumb=$(echo $img | sed -e "s/images\/picpile\///")
			if [ ! -e "images/thumbs/th-${thumb}" ] ; then
#				echo "$img -> $thumb"
				convert -resize 100x100 "${img}" "images/thumbs/th-${thumb}"
			fi
		fi
	done

	for img in images/users/*.$ext ; do
		# when there are no matches, we get the odd result of echo saying the text
		# instead of returning a blank pattern match
		if [ "$img" != "images/users/*.$ext" ] ; then
			thumb=$(echo $img | sed -e "s/images\/users\///")
			if [ ! -e "images/users/thumbs/th-${thumb}" ] ; then
#				echo "$img -> $thumb"
				convert -resize 100x100 "${img}" "images/users/thumbs/th-${thumb}"
			fi
		fi
	done


done
