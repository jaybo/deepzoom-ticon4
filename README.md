# TICON-4 processing

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
./test.sh
cat new.head.txt TICON-4.txt > new.txt
tcd-utils/build_tide_db new.tcd new.txt new.xml
# new.tcd is the result
```
