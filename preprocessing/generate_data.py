import xml.etree.ElementTree as ET
import requests
import json

def read(filename):
    tree = ET.parse(filename)
    return tree.getroot()

def create_osrm_request(sources, destinations):
    all_coords = sources + destinations
    
    formatted_coords = ";".join([f"{lon},{lat}" for lat, lon in all_coords])
    
    source_indices = ";".join(str(i) for i in range(len(sources)))
    dest_indices = ";".join(str(i + len(sources)) for i in range(len(destinations)))
    
    base_url = "http://localhost:5000/table/v1/driving/"
    url = f"{base_url}{formatted_coords}?annotations=duration&sources={source_indices}&destinations={dest_indices}"
    return url, all_coords

def extract_coordinates_with_parking(root):
    parking_coords = []
    dropoff_coords = []

    node_dict = {}
    for node in root.findall('node'):
        lat = float(node.get('lat'))
        lon = float(node.get('lon'))
        coords = (lat, lon)
        node_id = node.get('id')
        node_dict[node_id] = (lat, lon)
        
        parking_types = ['parking', 'parking_space', 'parking_entrance']
        is_parking = False
        amenity_tag = node.find("tag[@k='amenity']")
        
        if amenity_tag is not None:
            amenity_value = amenity_tag.get('v')
            if amenity_value in parking_types:
                is_parking = True
        
        access_tag = node.find("tag[@k='access'][@v='yes']")
        is_employee_only = node.find("tag[@k='fee'][@v='Employees only']")
        capacity_tag = node.find("tag[@k='capacity']")
        
        if amenity_tag is not None:
            if (is_parking and 
                access_tag is not None and 
                capacity_tag is not None and 
                is_employee_only is None):
                parking_coords.append((coords, int(capacity_tag.get('v'))))
            elif not is_parking:
                dropoff_coords.append(coords)
                
    for way in root.findall('way'):
        parking_types = ['parking', 'parking_space', 'parking_entrance']
        is_parking = False
        amenity_tag = way.find("tag[@k='amenity']")
        if amenity_tag is not None:
            amenity_value = amenity_tag.get('v')
            if amenity_value in parking_types:
                is_parking = True

        has_access = (way.find("tag[@k='access'][@v='yes']")) is not None
        is_employee_only = (way.find("tag[@k='fee'][@v='Employees only']")) is not None
        capacity_tag = way.find("tag[@k='capacity']")
        has_capacity = capacity_tag is not None
        
        way_coords = []
        for nd in way.findall('nd'):
            node_id = nd.get('ref')
            if node_id in node_dict:
                coords = node_dict[node_id]
                way_coords.append(coords)
        
        if way_coords:
            middle_index = len(way_coords) // 2
            middle_coords = way_coords[middle_index]
            
            # Only add to parking if it's public parking with capacity and not employees only
            if is_parking and has_access and has_capacity and not is_employee_only:
                if (middle_coords in dropoff_coords):
                    dropoff_coords.remove(middle_coords)
                parking_coords.append((middle_coords, int(capacity_tag.get('v'))))
                
    coords_list = []
    capacities_list = []
    for coords, capacity in parking_coords:
        coords_list.append(coords)
        capacities_list.append(capacity)
    
    return dropoff_coords, coords_list, capacities_list

def calculate_shortest_roundtrips(dropoff_to_parking, parking_to_dropoff):
    print("Generating shortest round trips...")
    smallest_round_trips = []
    for i in range(len(dropoff_to_parking)):
        min_round_trip = 2**64 - 1
    
        for j in range(len(parking_to_dropoff)):
            round_trip_time = dropoff_to_parking[i][j] + parking_to_dropoff[j][i]
            
            if round_trip_time < min_round_trip:
                min_round_trip = round_trip_time
        
        smallest_round_trips.append(min_round_trip)
        
    return smallest_round_trips

def write_response_to_file(dropoff_to_parking, parking_to_dropoff, parking_capacities, dropoff_coords, parking_coords):
    filename = f"../data.json"

    dropoff_to_parking = [[round(d / 60) for d in row] for row in dropoff_to_parking]
    parking_to_dropoff = [[round(d / 60) for d in row] for row in parking_to_dropoff]

    smallest_round_trips = calculate_shortest_roundtrips(dropoff_to_parking, parking_to_dropoff)

    formatted_dropoff_coords = []
    for lat, lon in dropoff_coords:
        formatted_dropoff_coords.append({
            "latitude": lat,
            "longitude": lon
        })

    formatted_parking_coords = []
    for lat, lon in parking_coords:
        formatted_parking_coords.append({
            "latitude": lat,
            "longitude": lon
        })

    output = {
        "dropoff_to_parking": dropoff_to_parking,
        "parking_to_dropoff": parking_to_dropoff,
        "parking_capacities": parking_capacities,
        "dropoff_coords": formatted_dropoff_coords,
        "parking_coords": formatted_parking_coords,
        "smallest_round_trips": smallest_round_trips
    }
    
    with open(filename, 'w') as f:
        json.dump(output, f)

def main():
    root = read("aalborg-map.osm")
    dropoff_coords, parking_coords, parking_capacities = extract_coordinates_with_parking(root)
    
    print(f"Found {len(dropoff_coords)} dropoff nodes and {len(parking_coords)} parking nodes")

    # First request: dropoff -> parking
    forward_url, _ = create_osrm_request(dropoff_coords, parking_coords)
    
    # Second request: parking -> dropoff
    reverse_url, _ = create_osrm_request(parking_coords, dropoff_coords)

    try:
        forward_response = requests.get(forward_url)
        forward_response.raise_for_status()
        forward_data = forward_response.json()
        
        reverse_response = requests.get(reverse_url)
        reverse_response.raise_for_status()
        reverse_data = reverse_response.json()
 
        write_response_to_file(forward_data['durations'], 
                               reverse_data['durations'],
                               parking_capacities,
                               dropoff_coords,
                               parking_coords)
        print("Responses written to file successfully")
        
    except requests.exceptions.RequestException as e:
        print(f"Error making request")

if __name__ == "__main__":
    main()