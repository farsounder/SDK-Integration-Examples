from __future__ import annotations

import argparse
import time

from farsounder import config, requests, subscriber
from farsounder.proto import nav_api_pb2


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Minimal Python SDK example.")
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
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    cfg = config.build_config(
        host=args.host,
        subscribe=["TargetData"],
    )

    sub = subscriber.subscribe(cfg)

    def on_targets(message: nav_api_pb2.TargetData) -> None:
        print("Got a TargetData message")
        print("Target groups:", len(message.groups))
        print("Bottom bins:", len(message.bottom))

    sub.on("TargetData", on_targets)

    print(f"Connecting to SonaSoft at {args.host}")
    print("Requesting processor settings...")
    settings = requests.get_processor_settings(cfg)
    print(settings)

    print(f"Listening for pub/sub updates for {args.seconds:.1f} seconds...")
    sub.start()
    try:
        time.sleep(args.seconds)
    finally:
        sub.stop()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
