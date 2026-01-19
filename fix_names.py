import gzip
import shutil
import os, sys
import json
import time
import geojson
import math
import chardet

json_file = "TICON-4.json"
out_clear = "/mnt/e/TideFiles/y25nf/y25nf.all.text.geojson"
out_gzip = "/mnt/e/TideFiles/y25nf/y25nf.all.geojson"


def write_output(stations):
    with open(out_clear, "w", encoding="utf-8") as f:
        f.write(json.dumps(stations, indent=2))

    with open(out_clear, "rb") as f_in:
        with gzip.open(out_gzip, "wb") as f_out:
            shutil.copyfileobj(f_in, f_out)


def read_out_json():
    with open(out_clear, "r", encoding="utf-8") as f:
        stations = json.load(f)
    return stations


def read_in_json():
    with open(json_file, "r", encoding="utf-8") as f:
        stations = json.load(f)
    return stations


# convoluted piece of shit.
# libtcd or somebody isn't handling Unicode.
# copy the final name string back into the output files.
def overwrite_station_names(stations_in, stations_out):
    match = 0
    for sout in stations_out["features"]:
        out_name = sout["properties"]["name"]
        for sin in stations_in:
            in_name = f"::{sin["tide_gauge_name"]}::"  
            if in_name in out_name:
                # match the tide_gauge_name INSIDE the :: separated name string
                match = match + 1
                sout["properties"]["name"] = sin["name"]
                if in_name == "iledaixtg_60minute-ile-fra-cmems":
                    k = 1
                break
    print("match", match)


if __name__ == "__main__":
    stations_in = read_in_json()
    print("in", len(stations_in))

    stations_out = read_out_json()
    print("out", len(stations_out["features"]))

    #
    overwrite_station_names(stations_in, stations_out)  

    write_output(stations_out)
