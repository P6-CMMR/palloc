import requests
import argparse

def fetch_osm_data(output_file, coordinates):
    overpass_url = "http://overpass-api.de/api/interpreter"
    min_lat, min_lon, max_lat, max_lon = coordinates
    bbox = f"{min_lat},{min_lon},{max_lat},{max_lon}"
    
    query = f"""
    [out:xml];
    (
      way["amenity"]({bbox});
      node["amenity"]({bbox});
    );
    out body;
    >;
    out skel qt;
    """
    
    try:
        print("Fetching OSM data...")
        response = requests.post(overpass_url, data=query)
        response.raise_for_status()
        
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(response.text)
            
        print(f"OSM data saved to {output_file}")
        return output_file
        
    except requests.exceptions.RequestException as e:
        print(f"Error fetching OSM data: {e}")
        return None

def main():
    parser = argparse.ArgumentParser(description="Fetch OSM data for a specified bounding box.")
    parser.add_argument("--city", "-c", type=str, default="Aalborg", help="City name")
    
    args = parser.parse_args()
    
    city_coordinates = {
        "Aalborg": (57.00120, 9.82520, 57.07639, 10.04768),
        "Aarhus": (56.12374,10.05816,56.20880,10.32766),
        "Copenhagen": (55.6276, 12.4640, 55.7302, 12.6480),
        "Odense": (55.3401, 10.2750, 55.4450, 10.5153)
    }
    
    output_file = f"{args.city.lower()}_map.osm"
    
    coordinates = city_coordinates.get(args.city)
    fetch_osm_data(output_file, coordinates)

if __name__ == "__main__":
    main()