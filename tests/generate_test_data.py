import json
 
# Data to be written
dictionary = {
"dropoff_to_parking": [[1, 2, 3], [4, 5, 6], [7, 8, 9]], 
"parking_to_dropoff": [[1, 2, 3], [4, 5, 6], [7, 8, 9]], 
"parking_capacities": [3, 5, 9], 
"dropoff_coords": [{"latitude": 11, "longitude": 12}, {"latitude": 14, "longitude": 15}, {"latitude": 16, "longitude": 17}], 
"parking_coords": [{"latitude": 56, "longitude": 57}, {"latitude": 58, "longitude": 59}, {"latitude": 60, "longitude": 61}], 
"smallest_round_trips": [2, 6, 10]
}
 
# Serializing json
json_object = json.dumps(dictionary, indent=4)
 
# Writing to sample.json
with open("test_data.json", "w") as outfile:
    outfile.write(json_object)