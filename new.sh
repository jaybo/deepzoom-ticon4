#!/bin/bash
cat new.head.txt TICON-4.txt > new.txt
tcd-utils/build_tide_db new.tcd new.txt new.xml

