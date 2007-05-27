#!/bin/bash

## surely there's a way to hit all the extensions in one pass,
## but i don't know what it is.

cd ~/themaxx.com/offensive/

for img in images/picpile/*.jpg
do
	thumb=$(echo $img | sed -e "s/images\/picpile\///") 
	if [ ! -e "images/thumbs/th-${thumb}" ]; then
		echo "$img -> $thumb"
		convert -sample 100x100 "${img}" "images/thumbs/th-${thumb}"
	fi
done

for img in images/picpile/*.JPG
do
	thumb=$(echo $img | sed -e "s/images\/picpile\///") 
	if [ ! -e "images/thumbs/th-${thumb}" ]; then
		echo "$img -> $thumb"
		convert -sample 100x100 "${img}" "images/thumbs/th-${thumb}"
	fi
done

for img in images/picpile/*.gif
do
	thumb=$(echo $img | sed -e "s/images\/picpile\///") 
	if [ ! -e "images/thumbs/th-${thumb}" ]; then
		echo "$img -> $thumb"
		convert -sample 100x100 "${img}" "images/thumbs/th-${thumb}"
	fi
done

for img in images/picpile/*.GIF
do
	thumb=$(echo $img | sed -e "s/images\/picpile\///") 
	if [ ! -e "images/thumbs/th-${thumb}" ]; then
		echo "$img -> $thumb"
		convert -sample 100x100 "${img}" "images/thumbs/th-${thumb}"
	fi
done

for img in images/picpile/*.png
do
	thumb=$(echo $img | sed -e "s/images\/picpile\///") 
	if [ ! -e "images/thumbs/th-${thumb}" ]; then
		echo "$img -> $thumb"
		convert -sample 100x100 "${img}" "images/thumbs/th-${thumb}"
	fi
done
