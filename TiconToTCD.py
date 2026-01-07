import pandas as pd
from timezonefinder import TimezoneFinder
from city_from_latlng import get_name_from_latlng
import country_converter as coco
import os, sys
import json
import time
import geojson
import math

csv_file = "TICON-4.csv"
json_file = "TICON-4.json"
geojson_file = "TICON-4.geojson"
txt_file = "TICON-4.txt"

tf = TimezoneFinder(in_memory=True)  # reuse
cc = coco.CountryConverter()

# tcd name : ticon name or None
tcd_cons = {
    "J1": "J1",
    "K1": "K1",
    "K2": "K2",
    "L2": "L2",
    "M1": "M1",
    "M2": "M2",
    "M3": "M3",
    "M4": "M4",
    "M6": "M6",
    "M8": "M8",
    "N2": "N2",
    "2N2": "2N2",
    "O1": "O1",
    "OO1": "OO1",
    "P1": "P1",
    "Q1": "Q1",
    "2Q1": "2Q1",
    "R2": "R2",
    "S1": "S1",
    "S2": "S2",
    "S4": "S4",
    "S6": "S6",
    "T2": "T2",
    "LDA2": "LM2",  # CHANGED
    "MU2": "MI2",  # CHANGED
    "NU2": "NI2",  # CHANGED
    "RHO1": "RHO1",
    "MK3": "MK3",
    "2MK3": "2MK3",
    "MN4": "MN4",
    "MS4": "MS4",
    "2SM2": "2SM2",
    "MF": "MF",
    "MSF": "MSF",
    "MM": "MM",
    "SA": "SA",
    "SSA": "SSA",
    "SA-IOS": "SA-IOS",
    "MF-IOS": "MF-IOS",
    "S1-IOS": "S1-IOS",
    "OO1-IOS": "OO1-IOS",
    "R2-IOS": "R2-IOS",
    "A7": "A7",
    "2MK5": "2MK5",
    "2MK6": "2MK6",
    "2MN2": "2MN2",
    "2MN6": "2MN6",
    "2MS6": "2MS6",
    "2NM6": "2NM6",
    "2SK5": "2SK5",
    "2SM6": "2SM6",
    "3MK7": "3MK7",
    "3MN8": "3MN8",
    "3MS2": "3MS2",
    "3MS4": "3MS4",
    "3MS8": "3MS8",
    "ALP1": "ALP1",
    "BET1": "BET1",
    "CHI1": "CHI1",
    "H1": "H1",
    "H2": "H2",
    "KJ2": "KJ2",
    "ETA2": "ETA2",
    "KQ1": "KQ1",
    "UPS1": "UPS1",
    "M10": "M10",
    "M12": "M12",
    "MK4": "MK4",
    "MKS2": "MKS2",
    "MNS2": "MNS2",
    "EPS2": "EP2",  # CHANGED
    "MO3": "MO3",
    "MP1": "MP1",
    "TAU1": "TAU1",
    "MPS2": "MPS2",
    "MSK6": "MSK6",
    "MSM": "MSM",
    "MSN2": "MSN2",
    "MSN6": "MSN6",
    "NLK2": "NLK2",
    "NO1": "NO1",
    "OP2": "OP2",
    "OQ2": "OQ2",
    "PHI1": "PHI1",
    "KP1": "KP1",
    "PI1": "PI1",
    "TK1": "TK1",
    "PSI1": "PSI1",
    "RP1": "RP1",
    "S3": "S3",
    "SIG1": "SIG1",
    "SK3": "SK3",
    "SK4": "SK4",
    "SN4": "SN4",
    "SNK6": "SNK6",
    "SO1": "SO1",
    "SO3": "SO3",
    "THE1": "THE1",
    "2PO1": "2PO1",
    "2NS2": "2NS2",
    "MLN2S2": "MLN2S2",
    "2ML2S2": "2ML2S2",
    "SKM2": "SKM2",
    "2MS2K2": "2MS2K2",
    "MKL2S2": "MKL2S2",
    "M2(KS)2": "M2(KS)2",
    "2SN(MK)2": "2SN(MK)2",
    "2KM(SN)2": "2KM(SN)2",
    "NO3": "NO3",
    "2MLS4": "2MLS4",
    "ML4": "ML4",
    "N4": "N4",
    "SL4": "SL4",
    "MNO5": "MNO5",
    "2MO5": "2MO5",
    "MSK5": "MSK5",
    "2MP5": "2MP5",
    "3MP5": "3MP5",
    "MNK5": "MNK5",
    "2NMLS6": "2NMLS6",
    "MSL6": "MSL6",
    "2ML6": "2ML6",
    "2MNLS6": "2MNLS6",
    "3MLS6": "3MLS6",
    "2MNO7": "2MNO7",
    "2NMK7": "2NMK7",
    "2MSO7": "2MSO7",
    "MSKO7": "MSKO7",
    "2MSN8": "2MSN8",
    "2(MS)8": "2(MS)8",
    "2(MN)8": "2(MN)8",
    "2MSL8": "2MSL8",
    "4MLS8": "4MLS8",
    "3ML8": "3ML8",
    "3MK8": "3MK8",
    "2MSK8": "2MSK8",
    "2M2NK9": "2M2NK9",
    "3MNK9": "3MNK9",
    "4MK9": "4MK9",
    "3MSK9": "3MSK9",
    "4MN10": "4MN10",
    "3MNS10": "3MNS10",
    "4MS10": "4MS10",
    "3MSL10": "3MSL10",
    "3M2S10": "3M2S10",
    "4MSK11": "4MSK11",
    "4MNS12": "4MNS12",
    "5MS12": "5MS12",
    "4MSL12": "4MSL12",
    "4M2S12": "4M2S12",
    "M1C": "M1C",
    "3MKS2": "3MKS2",
    "OQ2-HORN": "OQ2-HORN",
    "MSK2": "MSK2",
    "MSP2": "MSP2",
    "2MP3": "2MP3",
    "4MS4": "4MS4",
    "2MNS4": "2MNS4",
    "2MSK4": "2MSK4",
    "3MN4": "3MN4",
    "2MSN4": "2MSN4",
    "3MK5": "3MK5",
    "3MO5": "3MO5",
    "3MNS6": "3MNS6",
    "4MS6": "4MS6",
    "2MNU6": "2MNU6",
    "3MSK6": "3MSK6",
    "MKNU6": "MKNU6",
    "3MSN6": "3MSN6",
    "M7": "M7",
    "2MNK8": "2MNK8",
    "2(MS)N10": "2(MS)N10",
    "MNUS2": "MNUS2",
    "2MK2": "2MK2",
    "3KM5": "3KM5",
    "KJ2-IHO": "KJ2-IHO",
}


def add_station_name(station):
    lat = station["lat"]
    lon = station["lon"]
    country_from_ISO = station["country_from_ISO"]

    # name = f"{lat}, {lon}, {country_from_ISO}"
    name = get_name_from_latlng(lat, lon, country_from_ISO)

    print(name)

    station["name"] = name


def write_csv(stations):
    txt = ""

    for station in stations:

        # testing only
        # if not "Seattle" in station["name"]:
        #     continue

        txt += "#\n"
        txt += "# BEGIN HOT COMMENTS\n"
        txt += f"# country: {station["country"]}\n"
        txt += f"# source: {station["gesla_source"]}\n"
        txt += "# restriction: Public Domain\n"
        txt += f"# station_id_context: {station["gesla_source"]}\n"
        txt += f"# station_id: {station["tide_gauge_name"]}\n"
        txt += "# date_imported: 20241228\n"
        txt += f"# datum: {station["datum_name"]}\n"
        txt += "# confidence: 10\n"
        txt += "# !units: meters\n"
        txt += f"# !longitude: {station["lon"]}\n"
        txt += f"# !latitude: {station["lat"]}\n"
        txt += f"{station["name"]}\n"
        txt += f"+00:00 :{station["tz"]}\n"  # hmm, where did the colon go?
        txt += f"{station["datum_value"]} meters\n"
        # J1              0.0400  237.60
        # K1
        for con in tcd_cons:
            # get the ticon name of the constituent
            con_ticon = tcd_cons[con]
            if con_ticon is None:
                txt += "x 0 0\n"
            else:
                station_val = station.get(con_ticon)
                if station_val is None:
                    txt += "x 0 0\n"
                else:
                    amp = station_val["amp"] / 100  # cm to m
                    pha = station_val["pha"]
                    pha = (pha + 360) % 360  # 0 to 360
                    txt += f"{con:10}  {amp:10.4f}  {pha:6.2f}\n"

    print(txt)
    with open("TICON-4.txt", "w", encoding='utf-8') as f:
        f.write(txt)


def stats():
    # Read TICON-4 data
    df = pd.read_csv("data/TICON-4.csv", sep=",")
    pd.set_option("display.max_rows", None)

    datum_information = (
        df.groupby(["lat", "lon", "datum_information"]).size().reset_index()
    )
    datum_counts = datum_information["datum_information"].value_counts()

    # Count datums
    print("TICON-4 Datums:")
    print(datum_counts)
    print(f"\nTotal unique stations: {len(datum_information)}")
    print(f"Total datums: {len(datum_counts)}")


def TICON_txt_to_json():
    # Read TICON-4 data
    df = pd.read_csv("data/TICON-4.csv", sep=",")

    # columns
    # ['lat', 'lon', 'con', 'amp', 'pha', 'amp_std', 'pha_std', 'missing_obs',
    #       'no_of_obs', 'years_of_obs', 'start_date', 'end_date', 'gesla_source',
    #       'tide_gauge_name', 'type', 'country', 'record_quality',
    #       'datum_information']

    countries_to_ignore = ["USA"]
    record_quality_to_include = ["No obvious issues"]
    include_seattle = False

    stations = []
    station_index = 0

    for ll, st in df.groupby(["lat", "lon"]):
        country = st["country"].to_list()[0]

        lat = st["lat"].to_list()[0]
        lon = st["lon"].to_list()[0]
        no_of_obs = st["no_of_obs"].to_list()[0]
        years_of_obs = st["years_of_obs"].to_list()[0]
        start_date = st["start_date"].to_list()[0]
        end_date = st["end_date"].to_list()[0]
        gesla_source = st["gesla_source"].to_list()[0]
        tide_gauge_name = st["tide_gauge_name"].to_list()[0]
        gauge_type = st["type"].to_list()[0]
        record_quality = st["record_quality"].to_list()[0]
        datum_information = st["datum_information"].to_list()[0]

        if country in countries_to_ignore:
            if include_seattle and "seattle" in tide_gauge_name:
                # include seattle just for ease in testing
                pass
            else:
                continue

        if record_quality not in record_quality_to_include:
            continue

        cons = st["con"].to_list()
        amps = st["amp"].to_list()
        phas = st["pha"].to_list()

        tz = tf.timezone_at(lng=lon, lat=lat)
        country_from_ISO = cc.convert(names=country, to="name_short")

        # name = st[""]
        station = {
            "index": station_index,
            "lat": lat,
            "lon": lon,
            "name": "",
            "no_of_obs": no_of_obs,
            "years_of_obs": years_of_obs,
            "start_date": start_date,
            "end_date": end_date,
            "gesla_source": gesla_source,
            "tide_gauge_name": tide_gauge_name,
            "gauge_type": gauge_type,
            "country": country,
            "country_from_ISO": country_from_ISO,
            "record_quality": record_quality,
            "tz": tz,
            "datum_information": datum_information,
            "datum_name": "",
            "datum_value": 0.0,
        }
        for index, con in enumerate(cons):
            amp = amps[index]
            pha = phas[index]
            station[con] = {"amp": amp, "pha": pha}

        stations.append(station)
        station_index = station_index + 1

    return stations


# one time operation, add the gauge name to the datums
def add_tide_gauge_names_to_datums(stations):
    with open("data/y25nf.datums.json", "r", encoding='utf-8') as f:
        station_datums = json.load(f)
    for index, station in enumerate(stations):
        station_datums[index]["tide_gauge_name"] = station["tide_gauge_name"]
    with open("data/y25nf.datums.json", "w", encoding='utf-8') as f:
        f.write(json.dumps(station_datums))


def add_station_datums(stations):
    with open("data/y25nf.datums.json", "r", encoding='utf-8') as f:
        station_datums = json.load(f)

    for station in stations:
        datum = "LAT"
        match station["datum_information"]:
            case "USGS Station Datum (see station page for tie to geocentric datum)":
                datum = "MLLW"
            case (
                "MSL"
                | "Normal Amsterdam Level"
                | "RH 2000 (Swedish National Height System 2000)"
                | "BSCD2000"
                | "DVR90"
            ):
                datum = "MSL"
            case _:
                pass
        # datums
        tide_gauge_name = station["tide_gauge_name"]
        datum_match = [
            item
            for item in station_datums
            if item["tide_gauge_name"] == tide_gauge_name
        ]
        if len(datum_match) == 1:
            station["datum_name"] = datum
            station["datum_value"] = -datum_match[0][datum]
        else:
            print("fail", len(datum_match))


def write_json(stations):
    with open(json_file, "w", encoding='utf-8') as f:
        f.write(json.dumps(stations))
    return stations


def read_json():
    with open(json_file, "r", encoding='utf-8') as f:
        stations = json.load(f)
    return stations


def add_station_names(stations, redo=False):
    if redo:
        for station in stations:
            station["name"] = None

    for station in stations:
        if not station.get("name", None):
            add_station_name(station)
            write_json(stations)  # checkpoint
            time.sleep(1.25)


def write_geojson(stations):
    out = []
    for index, station in enumerate(stations):
        pt = geojson.Point([station["lon"], station["lat"]])
        out.append(geojson.Feature(index, pt, {"name": station["name"]}))
    fc = geojson.FeatureCollection(out)
    with open(geojson_file, "w", encoding='utf-8') as f:
        f.write(json.dumps(fc))


if __name__ == "__main__":
    redoTICON = False
    redoNames = True

    # stats()

    # create the list from scratch
    if redoTICON:
        stations = TICON_txt_to_json()
        write_json(stations)
        print(len(stations))

    # all stations as json
    stations = read_json()
    print(len(stations))

    # add the station name if it doesn't exist
    add_station_names(stations, redoNames)  # force if redoNames

    # only do this once!
    # add_tide_gauge_names_to_datums(stations)

    # add the station datum
    add_station_datums(stations) 

    # make a geojson version just for visualization
    write_geojson(stations)

    # save as json
    stations = write_json(stations)  # with names appended

    # the point of it all, save as csv for build_tide_db
    write_csv(stations)
