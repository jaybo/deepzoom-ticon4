#!/bin/sh
set -e
if [ -z "$2" ]; then
  echo Usage: rewrite_tide_db infile outfile
  exit
fi
restore_tide_db "$1" deleteme
build_tide_db "$2" deleteme.txt deleteme.xml
rm deleteme.txt deleteme.xml
