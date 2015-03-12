#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Usage $0 catalog.po"
    echo "Examples:"
	echo "    $0 translations/fr/HeeksCAD.po"
    exit 1
fi

TMP_POT_FILE=/tmp/heekscad-catalog.pot
xgettext -C -k_ -o $TMP_POT_FILE src/*.cpp src/*.h interface/*.cpp interface/*.h ../heekscnc/src/*.cpp ../heekscnc/src/*.h && msgmerge --update $1 $TMP_POT_FILE && rm $TMP_POT_FILE
