from __future__ import annotations

import argparse
import math
import time
from datetime import datetime, timezone
from pathlib import Path

import numpy as np
import rasterio
import utm
from rasterio.crs import CRS
from rasterio.transform import from_origin

from farsounder import config, subscriber
from farsounder.proto import nav_api_pb2

NODATA = -9999.0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Subscribe to TargetData and write one GeoTIFF per ping."
    )
    parser.add_argument(
        "host",
        nargs="?",
        default="127.0.0.1",
        help="Host running SonaSoft (default: 127.0.0.1)",
    )
    parser.add_argument(
        "--seconds",
        type=float,
        default=5.0,
        help="How long to wait for pub/sub messages before stopping",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("output"),
        help="Directory for per-ping GeoTIFF files (default: output)",
    )
    parser.add_argument(
        "--resolution-m",
        type=float,
        default=4.0,
        help="Raster cell size in meters (default: 4.0)",
    )
    return parser.parse_args()


def has_valid_navigation(message: nav_api_pb2.TargetData) -> bool:
    nav_values = (
        message.position.lat,
        message.position.lon,
        message.heading.heading,
    )
    return all(math.isfinite(value) for value in nav_values)


def boat_position_to_utm(
    message: nav_api_pb2.TargetData,
) -> tuple[float, float, int, str]:
    easting, northing, zone_number, zone_letter = utm.from_latlon(
        message.position.lat,
        message.position.lon,
    )
    return easting, northing, zone_number, zone_letter


def bin_to_utm(
    boat_easting: float,
    boat_northing: float,
    heading_deg: float,
    cross_range: float,
    down_range: float,
) -> tuple[float, float] | None:
    values = (cross_range, down_range, heading_deg, boat_easting, boat_northing)
    if not all(math.isfinite(value) for value in values):
        return None

    heading_rad = math.radians(heading_deg)
    forward_m = down_range
    # Positive cross-range is port/left, so flip it into a conventional right axis.
    right_m = -cross_range
    east_offset = forward_m * math.sin(heading_rad) + right_m * math.cos(heading_rad)
    north_offset = forward_m * math.cos(heading_rad) - right_m * math.sin(heading_rad)
    return boat_easting + east_offset, boat_northing + north_offset


def format_timestamp(message_time) -> str:
    dt = datetime(
        year=message_time.year,
        month=message_time.month,
        day=message_time.day,
        hour=message_time.hour,
        minute=message_time.minute,
        second=message_time.second,
        microsecond=message_time.millisecond * 1000,
        tzinfo=timezone.utc,
    )
    return dt.strftime("%Y%m%d_%H%M%S_%f")[:-3]


def burn_max(
    band: np.ndarray,
    points: list[tuple[float, float, float]],
    min_x: float,
    max_y: float,
    resolution_m: float,
) -> None:
    height, width = band.shape
    for easting, northing, value in points:
        col = min(width - 1, max(0, int((easting - min_x) / resolution_m)))
        row = min(height - 1, max(0, int((max_y - northing) / resolution_m)))
        if band[row, col] == NODATA or value > band[row, col]:
            band[row, col] = value


def write_ping_geotiff(
    message: nav_api_pb2.TargetData,
    output_dir: Path,
    resolution_m: float,
) -> Path | None:
    if not has_valid_navigation(message):
        return None

    boat_easting, boat_northing, zone_number, zone_letter = boat_position_to_utm(
        message
    )
    heading_deg = message.heading.heading
    bottom_points: list[tuple[float, float, float]] = []
    target_points: list[tuple[float, float, float]] = []

    for bottom_bin in message.bottom:
        if not math.isfinite(bottom_bin.depth):
            continue

        xy = bin_to_utm(
            boat_easting,
            boat_northing,
            heading_deg,
            bottom_bin.cross_range,
            bottom_bin.down_range,
        )
        if xy is not None:
            bottom_points.append((xy[0], xy[1], -bottom_bin.depth))

    for group in message.groups:
        for target_bin in group.bins:
            if not math.isfinite(target_bin.strength):
                continue

            xy = bin_to_utm(
                boat_easting,
                boat_northing,
                heading_deg,
                target_bin.cross_range,
                target_bin.down_range,
            )
            if xy is not None:
                target_points.append((xy[0], xy[1], target_bin.strength))

    all_points = bottom_points + target_points
    if not all_points:
        return None

    eastings = [point[0] for point in all_points]
    northings = [point[1] for point in all_points]
    min_x = math.floor(min(eastings) / resolution_m) * resolution_m
    min_y = math.floor(min(northings) / resolution_m) * resolution_m
    max_x = math.ceil(max(eastings) / resolution_m) * resolution_m
    max_y = math.ceil(max(northings) / resolution_m) * resolution_m

    width = max(1, int(math.ceil((max_x - min_x) / resolution_m)))
    height = max(1, int(math.ceil((max_y - min_y) / resolution_m)))
    max_x = min_x + width * resolution_m
    max_y = min_y + height * resolution_m

    depth_band = np.full((height, width), NODATA, dtype=np.float32)
    signal_band = np.full((height, width), NODATA, dtype=np.float32)
    burn_max(depth_band, bottom_points, min_x, max_y, resolution_m)
    burn_max(signal_band, target_points, min_x, max_y, resolution_m)

    timestamp = format_timestamp(message.time)
    serial = message.serial or "unknown"
    output_path = output_dir / f"{timestamp}_{serial}.tif"
    epsg = (32600 if zone_letter >= "N" else 32700) + zone_number
    max_range = message.grid_description.max_range

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with rasterio.open(
        output_path,
        "w",
        driver="GTiff",
        dtype="float32",
        count=2,
        width=width,
        height=height,
        crs=CRS.from_epsg(epsg),
        transform=from_origin(min_x, max_y, resolution_m, resolution_m),
        nodata=NODATA,
    ) as dataset:
        dataset.write(depth_band, 1)
        dataset.write(signal_band, 2)
        dataset.update_tags(
            PROVIDER="SonaSoft",
            TIMESTAMP_UTC=timestamp,
            SERIAL=message.serial,
            VESSEL_HEADING_DEG=f"{message.heading.heading:.6f}",
            VESSEL_LAT_DEG=f"{message.position.lat:.8f}",
            VESSEL_LON_DEG=f"{message.position.lon:.8f}",
            CELL_SIZE_METERS=f"{resolution_m:g}",
            BOTTOM_SAMPLE_COUNT=str(len(bottom_points)),
            TARGET_SAMPLE_COUNT=str(len(target_points)),
            BAND1_DESCRIPTION="bottom_depth_positive_down",
            BAND2_DESCRIPTION="target_strength_db",
            LOOK_AHEAD_RANGE_METERS=f"{max_range:.6f}",
        )
        dataset.update_tags(1, BAND_NAME="bottom_depth_meters_positive_down")
        dataset.update_tags(2, BAND_NAME="target_strength_db")

    return output_path


def main() -> int:
    args = parse_args()
    if args.resolution_m <= 0:
        raise SystemExit("--resolution-m must be greater than zero")

    cfg = config.build_config(
        host=args.host,
        subscribe=["TargetData"],
    )

    sub = subscriber.subscribe(cfg)
    ping_count = 0

    def on_targets(message: nav_api_pb2.TargetData) -> None:
        nonlocal ping_count
        output_path = write_ping_geotiff(
            message,
            args.output_dir,
            args.resolution_m,
        )
        if output_path is None:
            print("Skipped ping with no valid navigation or samples")
            return

        ping_count += 1
        print(f"Wrote GeoTIFF #{ping_count}: {output_path}")

    sub.on("TargetData", on_targets)

    print(f"Connecting to SonaSoft at {args.host}")
    print(f"Writing GeoTIFFs to {args.output_dir.resolve()}")
    print(f"Raster resolution: {args.resolution_m:g} m")
    print(f"Listening for pub/sub updates for {args.seconds:.1f} seconds...")
    sub.start()
    try:
        time.sleep(args.seconds)
    finally:
        sub.stop()

    print(f"Finished. Wrote {ping_count} GeoTIFF(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
