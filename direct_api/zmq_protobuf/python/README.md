# Python ZeroMQ/Protobuf Example

This example shows the two basic direct ZeroMQ/Protobuf flows:

- request/reply using `GetProcessorSettings`
- pub/sub by waiting for one `TargetData` message

## Setup

This folder is set up as a `uv` project.

Install dependencies:

```bash
uv sync
```

Generate the Python protobuf bindings:

```bash
uv run build_protos.py
```

## Run

```bash
uv run main.py
```

Pass a host if SonaSoft is running elsewhere:

```bash
uv run main.py 192.168.1.10
```

## What This Example Uses

- Request/reply port: `60501`
- TargetData pub/sub port: `61502`
- Proto files: `../proto/*.proto`
