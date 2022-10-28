import requests, json
from geopy.geocoders import Nominatim

def geocoding_reverse(lat_lng_str): 
    geolocoder = Nominatim(user_agent = 'South Korea', timeout=None)
    address = geolocoder.reverse(lat_lng_str)

    return address

def current_location():
    here_req = requests.get("http://www.geoplugin.net/json.gp")

    if (here_req.status_code != 200):
        print("error")
    else:
        location = json.loads(here_req.text)
        addr = geocoding_reverse(str(location["geoplugin_latitude"]) +', '+ str(location["geoplugin_longitude"]))
        addr = geocoding_reverse("37.5112, 126.9741")

    return addr

def geocoding_reverse(lat_lng_str): 
    geolocoder = Nominatim(user_agent = 'South Korea', timeout=None)
    address = geolocoder.reverse(lat_lng_str)

    return address

addr = current_location()
f = open("location.txt", 'w')
f.write(str(addr))
f.close()