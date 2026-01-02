#!/bin/bash
cat new.head.txt TICON-4.txt > new.txt
rm new.tcd
tcd-utils/build_tide_db new.tcd new.txt new.xml

