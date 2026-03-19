from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

from grpc_tools import protoc  # pyright: ignore[reportMissingImports]


def main() -> int:
    root = Path(__file__).resolve().parent
    proto_root = root.parent
    proto_dir = proto_root / "proto"
    out_dir = root / "generated"
    proto_files = sorted(proto_dir.glob("*.proto"))

    if not proto_files:
        print(f"No .proto files found in {proto_dir}", file=sys.stderr)
        return 1

    shutil.rmtree(out_dir, ignore_errors=True)
    out_dir.mkdir(parents=True, exist_ok=True)

    proto_root_arg = proto_root.as_posix()
    out_dir_arg = out_dir.as_posix()
    proto_file_args = [
        proto_file.relative_to(proto_root).as_posix() for proto_file in proto_files
    ]
    command = [
        "grpc_tools.protoc",
        f"--proto_path={proto_root_arg}",
        f"--python_out={out_dir_arg}",
        *proto_file_args,
    ]

    print("Running:", " ".join(command))
    previous_cwd = Path.cwd()
    try:
        os.chdir(proto_root)
        result = protoc.main(command)
    finally:
        os.chdir(previous_cwd)
    if result != 0:
        print("Failed to generate Python protobuf bindings.", file=sys.stderr)
        return 1

    generated_proto_dir = out_dir / "proto"
    generated_proto_dir.mkdir(parents=True, exist_ok=True)
    (generated_proto_dir / "__init__.py").touch()

    print(f"Generated {len(proto_files)} protobuf modules in {generated_proto_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
