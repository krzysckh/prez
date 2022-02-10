#!/bin/sh

for i in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
do
	echo "	{"

	convert -background white -fill black -font CozetteVector -pointsize 100 label:"$i" /tmp/fnt.png
	img2txt -W 8 -H 8 -g 1000 -b 50 /tmp/fnt.png \
		| ansifilter \
		| tr '[:graph:]' 'a' \
		| sed 's/^/\t\t{"/g' \
		| sed 's/$/"},/g'

	echo "	},"

done
