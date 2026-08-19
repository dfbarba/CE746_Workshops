# Earthquake XML Data Parser

This annex provides the Python script used to parse earthquake events from a QuakeML XML file (original format given by USGS). It extracts specific location details, magnitude, and focal mechanism angles, then outputs them into a structured Pandas DataFrame to plot using pyGMT.

## Steps
* **Parses QuakeML:** Reads specific earthquake event data from `data/events_2025_2026.xml`.
* **Handles XML Namespaces:** Correctly maps and searches complex nested XML tags.
* **Converts Units:** Automatically changes earthquake depth from meters to kilometers.
* **Extracts Key Metrics:** Pulls out essential geographical and seismological data fields.

## Extracted Data Fields
The script filters out events without focal mechanisms and saves the following metrics:
* `event_name`: Description text of the event.
* `longitude` / `latitude`: Geographical location coordinates.
* `depth`: Earthquake depth (converted to kilometers).
* `magnitude`: Size of the earthquake.
* `strike` / `dip` / `rake`: Angles defining the orientation of the fault rupture plane.

## Requirements
You need Python installed along with the Pandas library. Install Pandas using your terminal:

```bash
conda install -c conda-forge pandas
```

## Process
The script will process the XML data and print a clean, readable table (DataFrame) showing your formatted earthquake details.


```python
import xml.etree.ElementTree as ET
import pandas as pd

# Load XML file
tree = ET.parse("data/events_2025_2026.xml")
root = tree.getroot()

# Setup the exact namespaces from your file header
ns = {
    "q": "http://quakeml.org/xmlns/quakeml/1.2",  # For the root tag
    "": "http://quakeml.org/xmlns/bed/1.2",       # For all the internal data tags
}

events_data = []

# Find all <event> tags using the default namespace
for event in root.findall(".//event", ns):
    # Extract Name
    name = event.find("description/text", ns).text

    # Extract Origin Location Data
    origin = event.find("origin", ns)
    if origin is None:
        continue
        
    lon = float(origin.find("longitude/value", ns).text)
    lat = float(origin.find("latitude/value", ns).text)

    # Depth comes in meters from USGS, convert it to kilometers
    depth = float(origin.find("depth/value", ns).text) / 1000.0

    # Extract Earthquake Magnitude
    mag_elem = event.find("magnitude/mag/value", ns)
    mag = float(mag_elem.text) if mag_elem is not None else 0.0

    # Extract Strike, Dip, and Rake from the first Nodal Plane
    nodal_plane = event.find(".//nodalPlane1", ns)
    if nodal_plane is not None:
        strike = float(nodal_plane.find("strike/value", ns).text)
        dip = float(nodal_plane.find("dip/value", ns).text)
        rake = float(nodal_plane.find("rake/value", ns).text)
    else:
        continue # Skip the event if it doesn't have focal mechanism details

    # Store everything inside our data dictionary list
    events_data.append({
        "event_name": name,
        "longitude": lon,
        "latitude": lat,
        "depth": depth,
        "strike": strike,
        "dip": dip,
        "rake": rake,
        "magnitude": mag
    })

# Convert the list into a clean Pandas DataFrame
df = pd.DataFrame(events_data)
print(df)
```
