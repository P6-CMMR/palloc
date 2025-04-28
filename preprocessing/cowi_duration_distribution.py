count_map = {
    "gray": 99,
    "light_gray": 176,
    "orange": 308
}

# Eye read from figure 16 in cowi and approx rounded so every color adds to 1 across all buckets
percentage_buckets = [
    {
        "gray": 0.53,
        "light_gray": 0.12,
        "orange": 0.02
    }, 
    {
        "gray": 0.28,
        "light_gray": 0.23,
        "orange": 0.02
    }, 
    {
        "gray": 0.10,
        "light_gray": 0.25,
        "orange": 0.03
    }, 
    {
        "gray": 0.04,
        "light_gray": 0.08,
        "orange": 0.27
    }, 
    {
        "gray": 0.03,
        "light_gray": 0.27,
        "orange": 0.37
    }, 
    {
        "gray": 0.00,
        "light_gray": 0.03,
        "orange": 0.16
    }, 
    {
        "gray": 0.02,
        "light_gray": 0.02,
        "orange": 0.13
    }
]

gray_total_percentage = 0.0
light_gray_total_percentage = 0.0
orange_total_percentage = 0.0
for bucket in percentage_buckets:
    gray_total_percentage += bucket["gray"]
    light_gray_total_percentage += bucket["light_gray"]
    orange_total_percentage += bucket["orange"]

print("Total Percentages:")
print(f"Gray: {gray_total_percentage:.2f}")
print(f"Light Gray: {light_gray_total_percentage:.2f}")
print(f"Orange: {orange_total_percentage:.2f}")
print()

weighted_counts = []
for bucket in percentage_buckets:
    gray_percentage = bucket["gray"]
    light_gray_percentage = bucket["light_gray"]
    orange_percentage = bucket["orange"]

    gray_count = int(count_map["gray"] * gray_percentage)
    light_gray_count = int(count_map["light_gray"] * light_gray_percentage)
    orange_count = int(count_map["orange"] * orange_percentage)
    
    weighted_counts.append(gray_count + light_gray_count + orange_count)

total_count = sum(count_map.values())
merged_percentages = [count / total_count for count in weighted_counts]
    
scaled_percentages = [round(percentage, 2) for percentage in merged_percentages]

print("Scaled Percentages:")
for i, percentage in enumerate(scaled_percentages):
    print(f"Bucket {i + 1}: {percentage:.2f}")

sum_scaled_percentages = sum(scaled_percentages)
print(f"Sum of Scaled Percentages: {sum_scaled_percentages:.2f}")