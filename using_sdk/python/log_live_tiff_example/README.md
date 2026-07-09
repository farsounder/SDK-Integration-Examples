# Python SDK GeoTIFF Logger Example

Subscribe to `TargetData` and write one banded GeoTIFF per ping for ingestion in
QGIS, GDAL, or other GIS viewers.

Each output file contains two bands:

- Band 1: bottom depth from `message.bottom`
- Band 2: target strength from `message.groups[].bins` only

The example projects each ping into the UTM zone for the vessel position and
writes the correct projected CRS into the GeoTIFF metadata.

## Run

```bash
uv sync
uv run main.py # -h for args
```

To run without `uv`, install the dependencies from `pyproject.toml` and use:

```bash
python main.py
```

The canonical SDK repo is
[farsounder-python-client](https://github.com/farsounder/farsounder-python-client).