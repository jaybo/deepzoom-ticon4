from geopy.geocoders import Nominatim
import json

def get_city_from_latlng(latitude, longitude):
    """
    Performs reverse geocoding to get location details from lat/lng.
    """
    # Initialize Nominatim API with a custom user_agent
    # A unique user agent is required by the Nominatim usage policy
    #
    geolocator = Nominatim(timeout = 2.0, user_agent="deepzoom")
    
    # Combine latitude and longitude into a string format required by the reverse method
    coordinates = f"{latitude}, {longitude}"
    
    # Use the reverse method to get location details
    try:
        location = geolocator.reverse(coordinates, exactly_one=True)
        if location:
            # The location data is in a dictionary format within the 'raw' attribute
            address = location.raw.get('address', {})
            
            # Extract specific components
            city = address.get('city', address.get('town', address.get('village', '')))
            state = address.get('state', '')
            country = address.get('country', '')
            
            return city, state, country, location.address
        else:
            return None, None, None, None
    except Exception as e:
        return None, None, None, f"Error during geocoding: {e}"

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
