# from geopy.geocoders import Nominatim
import json
import requests

# Initialize Nominatim API with a custom user_agent
# A unique user agent is required by the Nominatim usage policy
#
# geolocator = Nominatim(timeout = 2.0, user_agent="deepzoom")
 
def get_name_from_latlng(latitude, longitude, country):
    """
    Performs reverse geocoding to get location details from lat/lng.
    """
    default = f"{latitude} {longitude}, {country}"

    response = requests.get(f"https://nominatim.openstreetmap.org/reverse?email=jay@deepzoom.com&format=geocodejson&lat={latitude}&lon={longitude}")

    def append_comma (name, val):
        if (len (name) > 0):
            return f"{name}, {val}"
        else:
            return val
        
    if response.status_code != 200:
        return default
    else:
        # Use the .json() method to parse the JSON content into a Python dictionary or list
        data = response.json()
        try:
            geocoding = data["features"][0]["properties"]["geocoding"]

            name = ""
            # street, district, city, state, country
            street = geocoding.get("street", None)
            district = geocoding.get("district", None)
            city = geocoding.get("city", None)
            state = geocoding.get("state", None)
            if street:
                name = street
            if district and district != street:
                name = append_comma(name, district)
            if city and city != district and city != street:
                name = append_comma(name, city)
            if state and state != city and state != district and state != street:
                name = append_comma(name, state)
            if (len(name) == 0):
                return default
            name = append_comma(name, country)
            # print (name)
            return name
        except (KeyError, TypeError):
            print ("Error")
            return default

    # # Combine latitude and longitude into a string format required by the reverse method
    # coordinates = f"{latitude}, {longitude}"
    
    # # Use the reverse method to get location details
    # try:
    #     location = geolocator.reverse(coordinates, exactly_one=True, addressdetails=True)
    #     if location:
    #         # The location data is in a dictionary format within the 'raw' attribute
    #         address = location.raw.get('address', {})
            
    #         # Extract specific components
    #         city = address.get('city', address.get('town', address.get('village', '')))
    #         state = address.get('state', '')
    #         country = address.get('country', '')
            
    #         return city, state, country, location.address
    #     else:
    #         return None, None, None, None
    # except Exception as e:
    #     return None, None, None, f"Error during geocoding: {e}"

if __name__ == "__main__":
    # Example Usage: Seattle, WA coordinates
    lat = 47.6062
    lng = -122.3321

    city, state, country, full_address = get_city_from_latlng(lat, lng)

    print(f"Latitude: {lat}, Longitude: {lng}")
    print(f"City: {city}")
    print(f"State: {state}")
    print(f"Country: {country}")
    print(f"Full Address: {full_address}")
