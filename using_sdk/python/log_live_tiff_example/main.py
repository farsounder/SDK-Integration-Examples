from __future__ import annotations

import argparse
import math
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path

import numpy as np
import rasterio
import utm
from rasterio.crs import CRS
from rasterio.transform import from_bounds

from farsounder import config, subscriber
from farsounder.proto import nav_api_pb2

NODATA = -9999.0


@dataclass(frozen=True)
class BottomSample:
    easting: float
    northing: float
    depth: float


@dataclass(frozen=True)
class TargetSample:
    easting: float
    northing: float
    strength: float


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
        default=1.0,
        help="Raster cell size in meters (default: 1.0)",
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


def rotate_boat_offset_to_world(
    forward_m: float,
    right_m: float,
    heading_deg: float,
) -> tuple[float, float]:
    heading_rad = math.radians(heading_deg)
    east_offset = forward_m * math.sin(heading_rad) + right_m * math.cos(heading_rad)
    north_offset = forward_m * math.cos(heading_rad) - right_m * math.sin(heading_rad)
    return east_offset, north_offset


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

    right_m = -cross_range
    east_offset, north_offset = rotate_boat_offset_to_world(
        forward_m=down_range,
        right_m=right_m,
        heading_deg=heading_deg,
    )
    return boat_easting + east_offset, boat_northing + north_offset


def extract_bottom_samples(message: nav_api_pb2.TargetData) -> list[BottomSample]:
    boat_easting, boat_northing, _, _ = boat_position_to_utm(message)
    heading_deg = message.heading.heading
    samples: list[BottomSample] = []

    for bottom_bin in message.bottom:
        if not math.isfinite(bottom_bin.depth):
            continue

        utm_xy = bin_to_utm(
            boat_easting,
            boat_northing,
            heading_deg,
            bottom_bin.cross_range,
            bottom_bin.down_range,
        )
        if utm_xy is None:
            continue

        samples.append(
            BottomSample(
                easting=utm_xy[0],
                northing=utm_xy[1],
                depth=bottom_bin.depth,
            )
        )

    return samples


def extract_target_samples(message: nav_api_pb2.TargetData) -> list[TargetSample]:
    boat_easting, boat_northing, _, _ = boat_position_to_utm(message)
    heading_deg = message.heading.heading
    samples: list[TargetSample] = []

    for group in message.groups:
        for target_bin in group.bins:
            if not math.isfinite(target_bin.strength):
                continue

            utm_xy = bin_to_utm(
                boat_easting,
                boat_northing,
                heading_deg,
                target_bin.cross_range,
                target_bin.down_range,
            )
            if utm_xy is None:
                continue

            samples.append(
                TargetSample(
                    easting=utm_xy[0],
                    northing=utm_xy[1],
                    strength=target_bin.strength,
                )
            )

    return samples


def snap_bounds(
    eastings: list[float],
    northings: list[float],
    resolution_m: float,
) -> tuple[float, float, float, float]:
    min_x = math.floor(min(eastings) / resolution_m) * resolution_m
    min_y = math.floor(min(northings) / resolution_m) * resolution_m
    max_x = math.ceil(max(eastings) / resolution_m) * resolution_m
    max_y = math.ceil(max(northings) / resolution_m) * resolution_m
    return min_x, min_y, max_x, max_y


def rasterize_ping(
    bottom_samples: list[BottomSample],
    target_samples: list[TargetSample],
    min_x: float,
    min_y: float,
    max_x: float,
    max_y: float,
    resolution_m: float,
) -> tuple[np.ndarray, np.ndarray]:
    width = int(round((max_x - min_x) / resolution_m))
    height = int(round((max_y - min_y) / resolution_m))
    depth_band = np.full((height, width), NODATA, dtype=np.float32)
    signal_band = np.full((height, width), NODATA, dtype=np.float32)

    for sample in bottom_samples:
        col = int((sample.easting - min_x) / resolution_m)
        row = int((max_y - sample.northing) / resolution_m)
        if not (0 <= row < height and 0 <= col < width):
            continue

        value = -sample.depth
        current = depth_band[row, col]
        if current == NODATA or value > current:
            depth_band[row, col] = value

    for sample in target_samples:
        col = int((sample.easting - min_x) / resolution_m)
        row = int((max_y - sample.northing) / resolution_m)
        if not (0 <= row < height and 0 <= col < width):
            continue

        current = signal_band[row, col]
        if current == NODATA or sample.strength > current:
            signal_band[row, col] = sample.strength

    return depth_band, signal_band


def utm_epsg(zone_number: int, zone_letter: str) -> int:
    if zone_letter >= "N":
        return 32600 + zone_number
    return 32700 + zone_number


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


def write_geotiff(
    output_path: Path,
    depth_band: np.ndarray,
    signal_band: np.ndarray,
    min_x: float,
    min_y: float,
    max_x: float,
    max_y: float,
    resolution_m: float,
    epsg: int,
    message: nav_api_pb2.TargetData,
    target_bin_count: int,
) -> None:
    height, width = depth_band.shape
    transform = from_bounds(min_x, min_y, max_x, max_y, width, height)
    timestamp = format_timestamp(message.time)

    tags = {
        "TIMESTAMP_UTC": timestamp,
        "SERIAL": message.serial,
        "HEADING_DEG": f"{message.heading.heading:.6f}",
        "LAT": f"{message.position.lat:.8f}",
        "LON": f"{message.position.lon:.8f}",
        "RESOLUTION_M": f"{resolution_m:g}",
        "BOTTOM_BIN_COUNT": str(len(message.bottom)),
        "TARGET_BIN_COUNT": str(target_bin_count),
        "BAND1_DESCRIPTION": "bottom_depth_negative_down",
        "BAND2_DESCRIPTION": "target_strength_db",
    }

    profile = {
        "driver": "GTiff",
        "dtype": "float32",
        "count": 2,
        "width": width,
        "height": height,
        "crs": CRS.from_epsg(epsg),
        "transform": transform,
        "nodata": NODATA,
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with rasterio.open(output_path, "w", **profile) as dataset:
        dataset.write(depth_band, 1)
        dataset.write(signal_band, 2)
        dataset.update_tags(**tags)
        dataset.update_tags(1, BAND_NAME="bottom_depth")
        dataset.update_tags(2, BAND_NAME="target_strength")


def write_ping_geotiff(
    message: nav_api_pb2.TargetData,
    output_dir: Path,
    resolution_m: float,
) -> Path | None:
    if not has_valid_navigation(message):
        return None

    bottom_samples = extract_bottom_samples(message)
    target_samples = extract_target_samples(message)
    if not bottom_samples and not target_samples:
        return None

    all_eastings = [sample.easting for sample in bottom_samples] + [
        sample.easting for sample in target_samples
    ]
    all_northings = [sample.northing for sample in bottom_samples] + [
        sample.northing for sample in target_samples
    ]
    min_x, min_y, max_x, max_y = snap_bounds(all_eastings, all_northings, resolution_m)

    depth_band, signal_band = rasterize_ping(
        bottom_samples,
        target_samples,
        min_x,
        min_y,
        max_x,
        max_y,
        resolution_m,
    )

    _, _, zone_number, zone_letter = boat_position_to_utm(message)
    epsg = utm_epsg(zone_number, zone_letter)
    timestamp = format_timestamp(message.time)
    serial = message.serial or "unknown"
    output_path = output_dir / f"{timestamp}_{serial}.tif"

    write_geotiff(
        output_path=output_path,
        depth_band=depth_band,
        signal_band=signal_band,
        min_x=min_x,
        min_y=min_y,
        max_x=max_x,
        max_y=max_y,
        resolution_m=resolution_m,
        epsg=epsg,
        message=message,
        target_bin_count=len(target_samples),
    )
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
