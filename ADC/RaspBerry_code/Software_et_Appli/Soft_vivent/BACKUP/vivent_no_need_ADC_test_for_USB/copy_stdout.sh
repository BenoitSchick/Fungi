#!/bin/bash
IFS=$'\n'
set -o noglob
USB=($(ls -a /media/pi/))
for i in "${!USB[@]}"
do
	if [ '.' != "${USB[i]}" ] && [ '..' != "${USB[i]}" ] 
	then
		path="/media/pi/${USB[i]}/stdout.txt"
		echo "stdout.txt has been copied in $path"
		cp stdout.txt $path
	fi
done
