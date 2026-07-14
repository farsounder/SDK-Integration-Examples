# C# ZeroMQ/Protobuf Example

This .NET 10 example shows the two basic direct ZeroMQ/Protobuf flows:

- request/reply using `GetProcessorSettings`
- pub/sub by waiting for one `TargetData` message

SonaSoft SDK Demo or a live SonaSoft installation must be running with the
ZeroMQ/Protobuf API enabled in order to receive data.

## Build

Install the [.NET 10 SDK](https://dotnet.microsoft.com/download/dotnet/10.0),
then run:

```bash
dotnet restore
dotnet build
```

The build uses `Grpc.Tools` to generate C# sources from all shared `.proto`
files. `Google.Protobuf` provides serialization, and `NetMQ` provides the
ZeroMQ sockets.

## Run

```bash
dotnet run
```

Pass a host if SonaSoft is running elsewhere:

```bash
dotnet run -- 192.168.1.10
```
