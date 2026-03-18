# C++ ZeroMQ/Protobuf Example

This example shows the two basic direct ZeroMQ/Protobuf flows:

- request/reply using `GetProcessorSettings`
- pub/sub by waiting for one `TargetData` message

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

The CMake project fetches `protobuf`, `libzmq`, and `cppzmq` itself so the
example stays consistent across Windows and Linux.

## Run

```bash
./build/bin/sonasoft_direct_cpp # linux
./build/bin/Release/sonasoft_direct_cpp.exe # windows
```

Pass a host if SonaSoft is running elsewhere, eg:

```bash
./build/bin/sonasoft_direct_cpp 192.168.1.10
```

You can also set the `SONASOFT_HOST` env var.

## What This Example Uses

- Request/reply port: `60501`
- TargetData pub/sub port: `61502`
- Proto files: `../proto/*.proto`

## Notes

The build generates C++ sources for all `.proto` files under `../proto/` so the
example matches the full schema checked into this repo.

The protobuf dependency is pinned to `v33.5` to match the C++ SDK client repo.
