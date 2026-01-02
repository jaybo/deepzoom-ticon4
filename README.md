# TICON-4 processing

Converts TICON-4.txt file containing worldwide tide harmonics into the .tcd format used by XTide

## Build libtcd

```
cd libtcd
./configure
make
sudo make install
```

## Build tcd_utils

```
cd libtcd
./configure
make
sudo make install
```

## Getting tcd_utils to work
add to .bashrc
`export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib`

## Get python libs
`pip install -r requirements.txt`

## Break apart an existing .tcd

```
cp somerandom.tcd tcd-utils/out.tcd
cd tcd-utils
restore_tide_db out.tcd
# produces out.txt and out.xml

# manually strip contents of out.xml, then
cp out.xml ../new.xml
```

## Process TICON-4.txt

```
python test.py
# generates TICON-4.txt, TICON-4.geojson, TICON-4.json
```

## Create new .tcd

```
# ./test.sh
#!/bin/bash
cat new.head.txt TICON-4.txt > new.txt
rm harmonics-jab-20251229-nonfree.tcd
tcd-utils/build_tide_db harmonics-jab-20251229-nonfree.tcd new.txt new.xml
cp harmonics-jab-20251229-nonfree.tcd /mnt/e/TideFiles
rm /mnt/e/TideFiles/harmonics-jab-20251229-nonfree.tcd.json
```
