from __future__ import annotations

import argparse
import sys
from pathlib import Path

import zmq


GENERATED_DIR = Path(__file__).resolve().parent / "generated"
if str(GENERATED_DIR) not in sys.path:
    sys.path.insert(0, str(GENERATED_DIR))

try:
    from proto import nav_api_pb2
except ModuleNotFoundError as error:
    raise SystemExit(
        "Missing generated protobuf files. Run `uv run build_protos.py` first."
    ) from error


PROCESSOR_SETTINGS_REQ_PORT = 60501
TARGET_DATA_SUB_PORT = 61502


def endpoint(host: str, port: int) -> str:
    return f"tcp://{host}:{port}"


def get_processor_settings(context: zmq.Context, host: str) -> nav_api_pb2.GetProcessorSettingsResponse:
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.RCVTIMEO, 5000)
    socket.setsockopt(zmq.SNDTIMEO, 5000)
    socket.connect(endpoint(host, PROCESSOR_SETTINGS_REQ_PORT))

    request = nav_api_pb2.GetProcessorSettingsRequest()
    socket.send(request.SerializeToString())
    reply = socket.recv()

    response = nav_api_pb2.GetProcessorSettingsResponse()
    response.ParseFromString(reply)
    socket.close()
    return response


def receive_target_data(context: zmq.Context, host: str) -> nav_api_pb2.TargetData:
    socket = context.socket(zmq.SUB)
    socket.setsockopt_string(zmq.SUBSCRIBE, "")
    socket.setsockopt(zmq.RCVTIMEO, 10000)
    socket.connect(endpoint(host, TARGET_DATA_SUB_PORT))

    payload = socket.recv()
    message = nav_api_pb2.TargetData()
    message.ParseFromString(payload)
    socket.close()
    return message


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Minimal direct ZeroMQ/Protobuf SonaSoft example."
    )
    parser.add_argument(
        "host",
        nargs="?",
        default="127.0.0.1",
        help="Host running SonaSoft (default: 127.0.0.1)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    context = zmq.Context()

    try:
        print(f"Connecting to SonaSoft at {args.host}")

        response = get_processor_settings(context, args.host)
        print(f"GetProcessorSettings result code: {response.result.code}")
        if response.HasField("settings"):
            settings = response.settings
            print("Processor settings")
            print(f"  system_type: {settings.system_type}")
            print(f"  fov: {settings.fov}")
            print(f"  detect_bottom: {settings.detect_bottom}")
            print(
                "  squelch range: "
                f"{settings.min_inwater_squelch} - {settings.max_inwater_squelch}"
            )
            print(f"  current squelch: {settings.inwater_squelch}")

        print("\nWaiting for one TargetData message...")
        target_data = receive_target_data(context, args.host)
        print("TargetData update")
        print(f"  target groups: {len(target_data.groups)}")
        print(f"  bottom bins: {len(target_data.bottom)}")
        if target_data.HasField("heading"):
            print(f"  heading: {target_data.heading.heading}")
        if target_data.HasField("position"):
            print(
                "  position: "
                f"{target_data.position.lat}, "
                f"{target_data.position.lon}"
            )

        return 0
    except zmq.error.Again:
        print("Timed out waiting for SonaSoft data.", file=sys.stderr)
        return 1
    finally:
        context.term()


if __name__ == "__main__":
    raise SystemExit(main())
