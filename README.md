# TICON-4 processing

Converts TICON-4.txt file containing worldwide tide harmonics into the .tcd format used by XTide.

The following is for WSL2 Ubuntu 24.04 on Windows.

## Build libtcd

Location names and time zone string lengths have expanded.  
Every client which uses the resulting .tcd file will need these changes.

Edit `tcd.h.in`:
```
#define ONELINER_LENGTH      256 // was 128
```
Edit `tide_db_default.h`:
```
#define DEFAULT_TZFILE_SIZE  64  // was 30
```
Build libtcd 

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

This is a one-time operation to generate the header .txt segment, and can be ignored if the TCD header has not evolved further.

```
cp somerandom.tcd tcd-utils/out.tcd
cd tcd-utils
restore_tide_db out.tcd
# produces out.txt and out.xml

# manually strip contents of out.xml
# manually strip just header of out.txt into ../new.head.txt
cp out.xml ../new.xml
```

## Process TICON-4.txt

May involve multiple manual internal steps to generate datums, add names, etc. if/when TICON-4.txt changes.
```
python TiconToTCD.py
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

## fix the naes

Somewhere in the chain through libtcd and TideFileGenerator, the UTF-8 station
names are mangled into LATIN-1.  Recopy the correct versions. This only needs to 
be done once each time TideFileGenerator makes the 

```
python fix_names.py
# operates on \y25nf\y25fr.all.text.geojson to produce \y25fr\y25fr.all.geojson"
```

## Seattle
Using constituents from NOAA 
https://tidesandcurrents.noaa.gov/harcon.html?unit=0&timezone=0&id=9447130&name=Seattle&state=WA

https://tidesandcurrents.noaa.gov/datums.html?datum=MTL&units=1&epoch=0&id=9447130&name=Seattle&state=WA

```
HAT, 2.02814650535584
MHHW, 1.41757070556771
MHW, 1.17437443454325
MSL, -4.02870069105791E-05
MTL, 0.0149606392292466
MLW, -1.14924864668054
MLLW, -1.96420739018312
LAT, -3.34870314598084
HAT - LAT, 5.37684965133667
HAT - MTL, 2.01318586612659
MTL + LAT, -3.33374250675159
MHHW - MLLW, 3.38177809575082
MTL - MLLW, 1.97916802941236
```