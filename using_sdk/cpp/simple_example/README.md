# C++ SDK Simple Example

This is the simplest C++ SDK example. It uses the higher-level `farsounder`
client library instead of managing ZeroMQ and protobuf directly.

## Build

This example fetches the packaged SDK release with CMake, matching the approach
used in the standalone example repo.

```bash
cmake -S . -B build
cmake --build build --config Release
```

On Windows, the CMake build also copies `farsounder.dll` next to the example
executable after the build.

## Run

```bash
./build/sonasoft_sdk_cpp_simple_example # linux
./build/Release/sonasoft_sdk_cpp_simple_example.exe # windows
```

You can pass a host if SonaSoft is running elsewhere:

```bash
./build/sonasoft_sdk_cpp_simple_example 192.168.1.10
```

## What It Demonstrates

- building a SDK config
- making a request/reply call with `get_processor_settings`
- subscribing to `TargetData`
- stopping the subscriber cleanly

The C++ SDK repo is
[farsounder-cpp-client](https://github.com/farsounder/farsounder-cpp-client).
