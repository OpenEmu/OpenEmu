#!/bin/sh

# Create the directory structure
# Avoided bash shortcuts / file-seperators in interest of portability

if [ -e testbed ]; then
	echo "Game-data already present as testbed/"
	echo "To regenerate, remove and rerun"
	exit 0
fi

mkdir testbed

cd testbed

# For game detection
echo "ScummVM rocks!" > TESTBED

mkdir test1
mkdir Test2
mkdir TEST3
mkdir tEST4
mkdir test5


cd test1
echo "It works!" > file.txt
cd ..

cd Test2
echo "It works!" > File.txt
cd ..

cd TEST3
echo "It works!" > FILE.txt
cd ..

cd tEST4
echo "It works!" > fILe.txt
cd ..

cd test5
echo "It works!" > file.
cd ..

# back to the top
cd ..

# move the audiocd data to newly created directory
cp -r testbed-audiocd-files testbed/audiocd-files
mv testbed/audiocd-files/music.mid testbed/
echo "Game data created"
