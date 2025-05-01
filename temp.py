data = []

# Read the temperature CSV file
with open("temperature.csv", "r") as file:
    lines = file.readlines()[1:]  # Skip header

    for line in lines:
        parts = line.strip().split(",")  # <-- Use ',' separator
        if len(parts) < 2:
            continue
        try:
            value = float(parts[1])  # 'value' is the second column
            data.append(value)
        except ValueError:
            continue  # Skip if value is not a valid float

# Manually calculate total, min, max
if not data:
    print("No data available to process.")
else:
    total = 0
    min_val = data[0]
    max_val = data[0]

    for val in data:
        total += val
        if val < min_val:
            min_val = val
        if val > max_val:
            max_val = val

    # Calculate average
    avg = total / len(data)

    # Print results
    print("Average Temperature:", avg)
    print("Minimum Temperature:", min_val)
    print("Maximum Temperature:", max_val)
