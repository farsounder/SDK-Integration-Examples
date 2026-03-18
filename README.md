# SonaSoft Integration Examples

This repository is the examples home for integrating with SonaSoft. The general
idea is that there are direct APIs you can use if you want more fine grained
control, and SDKs available to get you started quickly.

Either way - to actually get data you will need either the `SonaSoft SDK Demo`
running, or normal `SonaSoft` running connected to and an Argos sonar.

You can get the demo package by emailing us: `sw@farsounder.com` - it is available
free of charge for development or academic purposes. There is a licensing fee per
installation when you deploy your integration on customer vessels.

## Start Here

If you want the easiest path to accessing data, start with the SDK examples:

- `using_sdk/cpp/simple_example/`
- `using_sdk/python/simple_example/`
- `using_sdk/python/drawing_example/`

If you want to integrate with the APIs directly for full control of receive
threading, etc:

- `direct_api/zmq_protobuf/cpp/`
- `direct_api/zmq_protobuf/python/`
- Interactive Swagger docs at `http://localhost:3000/docs` when the SonaSoft demo is running

If you're having a problem with protobuf or zeromq and you want to test one piece at a time:

- `reference/zmq_protobuf_basics/`

## Choosing A Path

### Use The SDK

Use the SDK examples when you want the fastest route to request/reply calls and
streaming data without owning the lower-level transport details yourself. They
expose get/set functions for the request/reply calls - and let you register a
callback to run on new data from the pub/sub streams.

For reference, the SDK clients themselves live in their own repos:

- C++ client: [farsounder-cpp-client](https://github.com/farsounder/farsounder-cpp-client)
- Python client: [farsounder-python-client](https://github.com/farsounder/farsounder-python-client)

## Other Clients
- The Technische Universität Berlin has created a ROS2 package for interfacing with SonaSoft™ via the ZeroMQ/Protobuf API. You can find it [here](https://git.tu-berlin.de/farsounder_directories)

### Use The Direct APIs

SonaSoft exposes two direct APIs:

- [Protobuf](https://protobuf.dev/overview/) messages over [ZeroMQ](https://zeromq.org/) sockets
- JSON over HTTP or Websockets - eg a ["REST"](https://htmx.org/essays/how-did-rest-come-to-mean-the-opposite-of-rest/) API

JSON over HTTP/Websocket is usually the easiest direct path just because it has
become so common-place and most languages have some first class support for it. The 
ZeroMQ/Protobuf option makes sense for highest performance.

### Use The Reference Examples

The `reference/` examples are intentionally small. They are meant to help you
answer questions like:

- Is ZeroMQ installed and working?
- Is protobuf generation working?
- Can I do a minimal request/reply round trip?
- Can I receive a pub/sub message at all?

They are taken from the ZMQ and protobuf docs.

## Comparison of Integration Options

| Category | SDK Clients | HTTP / JSON | ZeroMQ / Protobuf |
| --- | --- | --- | --- |
| Best for | Fastest integration | Simple direct access | Lower-level direct access |
| Setup complexity | Lowest | Low | Highest |
| Transport details | Abstracted away | HTTP/Websocket | ZeroMQ sockets |
| Data format | Native client types | JSON | Protocol Buffers |
| Processor Controls | Yes | Yes | Yes |
| Live seafloor detections | Yes | Yes | Yes |
| Live in-water detections | Yes | Yes | Yes |
| Gridded history data | Yes | Yes | No |
| Hydrophone Timeseries Data | Yes | No | Yes |
| Third Party Targets (Sea.AI, etc) | No | Yes | No |
| SonaSoft LT Alarm Config | No | Yes | No |
| Filtered NMEA Stream Data | No | Yes | No |

## Notes

> [!NOTE]
> In 4.2.x, the HTTP and ZeroMQ APIs are mutually exclusive. To use the
> Protobuf/ZeroMQ API on 4.2.x, remove the `--http-nav-api` launcher argument
> from the SDK package. In 4.3+ they can be run together.

If you have questions or need access to the SDK package, contact
`sw@farsounder.com`.
