import json
import os
 
# Data to be written
dictionary = {
    "dropoff_to_parking": [[1, 2, 3], [4, 5, 6], [7, 8, 9]], 
    "parking_to_dropoff": [[1, 2, 3], [4, 5, 6], [7, 8, 9]], 
    "parking_capacities": [3, 5, 9], 
    "dropoff_coords": [{"lat": 11, "lon": 12}, {"lat": 14, "lon": 15}, {"lat": 16, "lon": 17}], 
    "parking_coords": [{"lat": 56, "lon": 57}, {"lat": 58, "lon": 59}, {"lat": 60, "lon": 61}], 
    "smallest_round_trips": [2, 6, 10]
}
 
# Serializing json
json_object = json.dumps(dictionary, indent=4)
 
# Writing to sample.json
if not os.path.exists("tests"):
    os.makedirs("tests")

filename = "tests/test_data.json"
with open(filename, "w") as outfile:
    outfile.write(json_object)

script_dir = os.getcwd()
output_path = os.path.join(script_dir, filename)
print(f"Data written to {output_path}")