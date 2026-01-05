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