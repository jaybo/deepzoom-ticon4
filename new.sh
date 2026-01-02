#!/bin/bash
cat new.head.txt TICON-4.txt > new.txt
rm harmonics-jab-20251229-nonfree.tcd
tcd-utils/build_tide_db harmonics-jab-20251229-nonfree.tcd new.txt new.xml
cp harmonics-jab-20251229-nonfree.tcd /mnt/e/TideFiles
rm /mnt/e/TideFiles/harmonics-jab-20251229-nonfree.tcd.json

