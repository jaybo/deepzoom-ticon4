import gzip
import shutil
import os, sys
import json
import time
import geojson
import math

json_file = "TICON-4.json"
out_clear = "/mnt/c/GitHub/TideFiles/y25fr/y25fr.all.text.geojson"
out_gzip = "/mnt/c/GitHub/TideFiles/y25fr/y25fr.all.geojson"


def write_output(stations):
    with open(out_clear, "w", encoding='utf-8') as f:
        f.write(json.dumps(stations))

    with open(out_clear, 'rb') as f_in:
        with gzip.open(out_gzip, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)

def read_out_json():
    with open(out_clear, "r", encoding='utf-8') as f:
        stations = json.load(f)
    return stations

def read_in_json():
    with open(json_file, "r", encoding='utf-8') as f:
        stations = json.load(f)
    return stations


def overwrite_station_names(stations_in, stations_out):
    pass
    # for sout in stations_out:
    #     station = 
    #     for sin in stations_in:
    #         if not sin.get("name", None):


if __name__ == "__main__":
    stations_in = read_in_json()
    print(len(stations_in))

    stations_out = read_out_json()
    print(len(stations_out))

    # add the station name if it doesn't exist
    overwrite_station_names(stations_in, stations_out)  # force if redoNames


    # write_output(stations_out)
