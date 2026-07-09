# Python SDK GeoTIFF Logger Example

Subscribe to `TargetData` and write one banded GeoTIFF per ping for ingestion in
QGIS, GDAL, or other GIS viewers.

Each output file contains two bands:

- Band 1: bottom depth from `message.bottom`, stored as negative-down values
- Band 2: target strength from `message.groups[].bins` only

The example projects each ping into the UTM zone for the vessel position and
writes the correct projected CRS into the GeoTIFF metadata.

## Run

```bash
uv sync
uv run main.py
```

Common options:

```bash
uv run main.py 192.168.1.10 --seconds 30 --output-dir output --resolution-m 2
```

- `host`: SonaSoft host (default `127.0.0.1`)
- `--seconds`: how long to listen for pings (default `5`)
- `--output-dir`: where to write `.tif` files (default `output`)
- `--resolution-m`: raster cell size in meters (default `1`)

Output files are named like `YYYYMMDD_HHMMSS_mmm_<serial>.tif`.

## Inspect Output

With GDAL:

```bash
gdalinfo output/<file>.tif
```

With Python:

```bash
uv run python -c "import rasterio; ds=rasterio.open('output/<file>.tif'); print(ds.crs, ds.transform, ds.tags())"
```

To run without `uv`, install the dependencies from `pyproject.toml` and use:

```bash
python main.py
```

The canonical SDK repo is
[farsounder-python-client](https://github.com/farsounder/farsounder-python-client).
