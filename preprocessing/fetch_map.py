import requests

def fetch_osm_data(coordinates):
    overpass_url = "http://overpass-api.de/api/interpreter"
    min_lat, min_lon, max_lat, max_lon = coordinates
    bbox = f"{min_lat},{min_lon},{max_lat},{max_lon}"
    
    query = f"""
    [out:xml];
    (
      way({bbox});
      node({bbox});
    );
    out body;
    >;
    out skel qt;
    """
    
    try:
        print("Fetching OSM data...")
        response = requests.post(overpass_url, data=query)
        response.raise_for_status()
        
        output_file = "aalborg-map.osm"
        with open(output_file, "w", encoding="utf-8") as f:
            f.write(response.text)
            
        print(f"OSM data saved to {output_file}")
        return output_file
        
    except requests.exceptions.RequestException as e:
        print(f"Error fetching OSM data: {e}")
        return None

def main():
    coordinates = (57.00120, 9.82520, 57.07639, 10.04768)
    fetch_osm_data(coordinates)

if __name__ == "__main__":
    main()