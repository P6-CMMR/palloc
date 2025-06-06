import xml.etree.ElementTree as ET
import requests
import json
import numpy as np
import argparse

from typing import cast
from sklearn.model_selection import GridSearchCV
from sklearn.neighbors import KernelDensity

def read_osm(filename: str):
    """Reads OSM file and returns the root element."""
    tree = ET.parse(filename)
    return tree.getroot()

def create_osrm_request(sources: list, destinations: list):
    """Creates a request URL for the OSRM API."""
    all_coords = sources + destinations
    
    formatted_coords = ";".join([f"{lon},{lat}" for lat, lon in all_coords])
    
    source_indices = ";".join(str(i) for i in range(len(sources)))
    dest_indices = ";".join(str(i + len(sources)) for i in range(len(destinations)))
    
    base_url = "http://localhost:5000/table/v1/driving/"
    url = f"{base_url}{formatted_coords}?annotations=duration&sources={source_indices}&destinations={dest_indices}"
    return url, all_coords

def extract_data_from_osm(root: ET.Element):
    """Extract data from OSM file"""
    parking_coords = []
    dropoff_coords = []

    node_dict = {}
    for node in root.findall("node"):
        lat_str = node.get("lat")
        lon_str = node.get("lon")
        
        if lat_str is None or lon_str is None:
            continue
        
        lat = float(lat_str)
        lon = float(lon_str)
        coords = (lat, lon)
        node_id = node.get("id")
        node_dict[node_id] = (lat, lon)
        
        parking_types = ["parking", "parking_space", "parking_entrance"]
        is_parking = False
        amenity_tag = node.find('tag[@k="amenity"]')
        
        if amenity_tag is not None:
            amenity_value = amenity_tag.get("v")
            if amenity_value in parking_types:
                is_parking = True
        
        access_tag = node.find('tag[@k="access"][@v="yes"]')
        has_employee_only = node.find('tag[@k="fee"][@v="Employees only"]') is not None
        capacity_tag = node.find('tag[@k="capacity"]')
        
        if amenity_tag is not None:
            if (is_parking and 
                access_tag is not None and 
                capacity_tag is not None and 
                not has_employee_only):
                capacity_str = cast(str, capacity_tag.get("v")) 
                parking_coords.append((coords, int(capacity_str)))
            elif not is_parking:
                dropoff_coords.append(coords)
                
    for way in root.findall("way"):
        parking_types = ["parking", "parking_space", "parking_entrance"]
        is_parking = False
        amenity_tag = way.find('tag[@k="amenity"]')
        if amenity_tag is not None:
            amenity_value = amenity_tag.get("v")
            if amenity_value in parking_types:
                is_parking = True

        has_access = way.find('tag[@k="access"][@v="yes"]') is not None
        has_employee_only = way.find('tag[@k="fee"][@v="Employees only"]') is not None
        capacity_tag = way.find('tag[@k="capacity"]')
        has_capacity = capacity_tag is not None
        
        way_coords = []
        for nd in way.findall("nd"):
            node_id = nd.get("ref")
            if node_id in node_dict:
                coords = node_dict[node_id]
                way_coords.append(coords)
        
        if way_coords:
            middle_index = len(way_coords) // 2
            middle_coords = way_coords[middle_index]
            
            # Only add to parking if it's public parking with capacity and not employees only
            if is_parking and has_access and has_capacity and not has_employee_only:
                if (middle_coords in dropoff_coords):
                    dropoff_coords.remove(middle_coords)
                assert capacity_tag is not None
                parking_coords.append((middle_coords, int(cast(str, capacity_tag.get("v")))))
                
    coords_list = []
    capacities_list = []
    for coords, capacity in parking_coords:
        coords_list.append(coords)
        capacities_list.append(capacity)
    
    return dropoff_coords, coords_list, capacities_list

def calculate_shortest_roundtrips(dropoff_to_parking: list, parking_to_dropoff: list):
    """Calculates the shortest round trip times for each dropoff to parking."""
    print("Generating shortest round trips...")
    smallest_round_trips = []
    for i in range(len(dropoff_to_parking)):
        min_round_trip = 2**32 - 1
        for j in range(len(parking_to_dropoff)):
            round_trip_time = dropoff_to_parking[i][j] + parking_to_dropoff[j][i]
            
            if round_trip_time < min_round_trip:
                min_round_trip = round_trip_time
        
        smallest_round_trips.append(min_round_trip)
        
    return smallest_round_trips

def calculate_parking_weight(dropoff_coords: list[tuple], parking_coords: list[tuple]):
    """Calculates the parking weight based on how many dropoff points are nearby."""
    print("Calculating parking weights based on dropoff points nearby...")
    
    GRID_SIZE = 100
    
    lats = np.array([lat for lat, _ in dropoff_coords])
    lons = np.array([lon for _, lon in dropoff_coords])
    
    min_lat, max_lat = np.min(lats), np.max(lats)
    min_lon, max_lon = np.min(lons), np.max(lons)
    
    lat_range = max_lat - min_lat
    lon_range = max_lon - min_lon
    
    lat_axis = np.linspace(min_lat, max_lat, GRID_SIZE)
    lon_axis = np.linspace(min_lon, max_lon, GRID_SIZE)
    
    lon_mesh, lat_mesh = np.meshgrid(lon_axis, lat_axis)
    positions = np.vstack([lat_mesh.ravel(), lon_mesh.ravel()])
    
    values = np.vstack([lats, lons])
    search_space = {"bandwidth": np.linspace(1e-3, 1e-2, 100)}

    search_grid = GridSearchCV(
        KernelDensity(kernel="gaussian"),
        search_space,
        n_jobs=-1, 
        cv=5
    )
    search_grid.fit(values.T)
    best_bandwidth = search_grid.best_params_["bandwidth"]
    
    print(f"Best KDE bandwidth found: {best_bandwidth}")
    
    log_density = search_grid.score_samples(positions.T) 
    density = np.exp(log_density)
    
    # Reshape into a grid
    density = density.reshape(GRID_SIZE, GRID_SIZE)
    density_max = np.max(density)
    if density_max > 0: 
        density_normalized = density / density_max
    else:
        density_normalized = density

    parking_weights = []
    for lat, lon in parking_coords:
        i = int(np.floor((lat - min_lat) / lat_range * (GRID_SIZE - 1)))
        j = int(np.floor((lon - min_lon) / lon_range * (GRID_SIZE - 1)))
        
        weight = 1 + density_normalized[i][j]
        parking_weights.append(weight)
        
    density_grid = []
    for i in range(GRID_SIZE):
        for j in range(GRID_SIZE):
            intensity = float(density_normalized[i][j])
            if intensity > 0.05:
                density_grid.append({
                    "lat": float(lat_mesh[i][j]),
                    "lon": float(lon_mesh[i][j]),
                    "intensity": intensity
                })

    return parking_weights, density_grid

def write_response_to_file(dropoff_to_parking: list, 
                           parking_to_dropoff: list, 
                           parking_capacities: list, 
                           dropoff_coords: list[tuple], 
                           parking_coords: list[tuple],
                           filename: str):
    """Writes a data file to JSON format."""

    dropoff_to_parking = [[round(d / 60) for d in row] for row in dropoff_to_parking]
    parking_to_dropoff = [[round(d / 60) for d in row] for row in parking_to_dropoff]

    formatted_dropoff_coords = []
    for lat, lon in dropoff_coords:
        formatted_dropoff_coords.append({
            "lat": lat,
            "lon": lon
        })

    formatted_parking_coords = []
    for lat, lon in parking_coords:
        formatted_parking_coords.append({
            "lat": lat,
            "lon": lon
        })
    
    smallest_round_trips = calculate_shortest_roundtrips(dropoff_to_parking, parking_to_dropoff)
    parking_weights, density_grid = calculate_parking_weight(dropoff_coords, parking_coords)

    output = {
        "dropoff_to_parking": dropoff_to_parking,
        "parking_to_dropoff": parking_to_dropoff,
        "parking_capacities": parking_capacities,
        "dropoff_coords": formatted_dropoff_coords,
        "parking_coords": formatted_parking_coords,
        "smallest_round_trips": smallest_round_trips,
        "parking_weights": parking_weights,
        "density_grid": density_grid,
    }
    
    with open(filename, "w") as f:
        json.dump(output, f)

def main():
    parser = argparse.ArgumentParser(description="Generate environment data from OSM file.")
    parser.add_argument("--map", "-m", type=str, default="aalborg_map.osm", help="OSM map file")
    args = parser.parse_args()
    
    root = read_osm(args.map)
    dropoff_coords, parking_coords, parking_capacities = extract_data_from_osm(root)
    
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
 
        output_file = "../" + args.map.replace("_map.osm", "_env.json")
        write_response_to_file(forward_data["durations"], 
                               reverse_data["durations"],
                               parking_capacities,
                               dropoff_coords,
                               parking_coords,
                               output_file)
        print("Responses written to file successfully")
        
    except requests.exceptions.RequestException as e:
        print(f"Error making request")

if __name__ == "__main__":
    main()