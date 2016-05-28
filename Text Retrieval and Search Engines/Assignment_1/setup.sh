#!/bin/bash

if [ ! -d ../meta ]
then
	echo "Error. Make sure the folder meta is in the same directory and rerun this script"
	exit 1
fi

echo "MeTA Found"

if [ ! -f ../meta/src/tools/profile.cpp ] && [ ! -f ../meta/src/index/tools/CMakeLists.txt ]
then
	echo "Error. Make sure you installed MeTA correctly."
	exit 1
fi

echo "Copying assignment 1 data files to MeTA..."

\cp -rf moocs ../meta/data/
if [ ! $? -eq 0 ]; then echo "Error. Make sure the script has enough writing privileges"; exit 1; fi

\cp -rf perceptron-tagger ../meta/data/
if [ ! $? -eq 0 ]; then echo "Error. Make sure the script has enough writing privileges"; exit 1; fi

\mkdir build
cp -rf Assignment1 build/

echo "Files copied successfully!"
exit 0
