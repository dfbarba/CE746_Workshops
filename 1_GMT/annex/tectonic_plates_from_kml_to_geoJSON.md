# Global Tectonic Plate Boundaries Map

## Data Source

* **Dataset:** USGS Global Tectonic Plate Boundaries 
* **Format:** Keyhole Markup Language (`.kml`)
* **Original Publisher:** United States Geological Survey (USGS)
* **Coordinate Reference System (CRS):** Geographic WGS84 (EPSG:4326), with coordinates mapped natively in Longitude/Latitude decimal degrees.

## Processing Transformations Performed

The following data-cleaning operations were executed:

* **HTML Table Scraping & Disaggregation:** The input `.kml` stores its complete relational database attributes inside a raw HTML `<table>` string block under a single `Description` text column. Regular Expressions (RegEx) were written to isolate key markers (`<td>LABEL</td>\s*<td>(.*?)</td>`), stripping out the nested boundaries data into a queryable Pandas dataframe column.
* **Geospatial Layer Segmentation:** The source file combines all line segments into one collection. The script splits the comprehensive dataframe into isolated datasets matching the three distinct geological structural types:
  * `Transform Boundary` (e.g., Strike-slip)
  * `Convergent Boundary` (e.g., Subduction zones)
  * `Divergent Boundary` (e.g., Mid-ocean rifts)
* **GeoJSON Conversion:** Rather than relying on custom OGR database drivers or transient coordinates, layers are compiled directly into independent **GeoJSON** standard files (`.geojson`). This allows PyGMT to read, reproject, and symbol-render the global geometries cleanly.

The script used to read the raw KML file, apply the RegEx text parser, split the boundaries by classification type, and save permanent, lightweight JSON documents under a local `/data_clean/` folder directory was:

```python
import os
import fiona
import geopandas as gpd

# Enable KML driver and read the raw file
fiona.drvsupport.supported_drivers["KML"] = "rw"
df = gpd.read_file("data/usgs_plate_boundaries.kml", driver="KML", engine="fiona")

# Extract clean attribute names from the embedded HTML table
df["LABEL"] = df["Description"].str.extract(r"<td>LABEL</td>\s*<td>(.*?)</td>")
df["LABEL"] = df["LABEL"].str.strip()

# Create an output directory for the clean GeoJSON layers
os.makedirs("data/plate_boundaries", exist_ok=True)

# Filter and save each boundary classification layer permanently
unique_labels = df["LABEL"].dropna().unique()

for label_name in unique_labels:
    sub_df = df[df["LABEL"] == label_name]
    
    # Generate a clean lowercase filename
    clean_filename = f"data/plate_boundaries/{label_name.replace(' ', '_').lower()}.geojson"
    
    # Save permanently to disk
    sub_df.to_file(clean_filename, driver="GeoJSON")
    print(f"Successfully saved: {clean_filename}")
```

## Final note:
## Remove lines 48-49 fom convergent_boundary.geojson
## Move line 54 from convergent_boundary.geojson to other.geojson