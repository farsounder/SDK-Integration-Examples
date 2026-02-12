# Minimal SonaSoft API Integation Example

## Two methods of communicating with SonaSoft™
There are two methods available to communicate with SonaSoft™. The first uses
messages that are serialized using Google's Protocol Buffers sent over ZeroMQ
sockets. The second uses an HTTP API that sends and receives JSON.

## Option 1: Protobuf/ZMQ API
The first option for integration is to use send and receive Protobuf messages over ZeroMQ. There are client libraries for both ZeroMQ and Protobuf in many 
different languages. And you can find specific information about our API in the
Interface Design Definition available on the
[SDK page of the website](https://www.farsounder.com/software-development-kit)

The necessary binaries for ZeroMQ on Windows are also included in the example.
However if you would like to download either of these dependencies to run
different versions based on your requirements - or to look at their official 
docs, you can find them here:
* ZeroMQ - https://zeromq.org/
* ProtocolBuffers - https://developers.google.com/protocol-buffers

We use ProtocolBuffers to manage structured data in a language neutral way, and ZeroMQ to pass serialized "proto" messages between processes.

The example for this method is in the zmq_protobuf_api folder. It is a simple
C++ program that connects to SonaSoft™ and receives messages from it, there is
an example for Ubuntu and Windows, and a dockerfile for running the example in
an Ubuntu container.

## Option 2: JSON over HTTP ["REST"](https://htmx.org/essays/how-did-rest-come-to-mean-the-opposite-of-rest/) API
The second option for integration is to use the JSON over HTTP API. This is a simple
API server that runs on the machine that SonaSoft™ is installed on. Sending
JSON over HTTP is much simpler than using ZeroMQ and Protobuf, but it is not
as fast or efficient. Live docs for the API are available on the machine that
SonaSoft™ is installed on, you can find them by starting SonaSoft and
going to http://localhost:3000/docs in your browser, or view them in the GH Pages
[for this repo](https://farsounder.github.io/SDK-Integration-Examples/).

> [!NOTE]
> In 4.2.x, these two APIs are mutually exclusive - it was assumed that
only one or the other would be used. The default was also switched to the
JSON over HTTP version. If running 4.2.x, in order to use the Protobuf/ZMQ
API, remove the "--http-nav-api" argument from the launcher in the SDK
package. This assumption is relaxed in an upcoming yet to be released version.
In 4.3+ any combination of them can be run together, at the cost of more
resources.

## Playing back recorded data w/ SonaSoft™ SDK
The FarSounderSDK package (can be obtained by contacting us if you do not have 
it already) contains an "SDK Version" of SonaSoft
that can be used to run through recorded sonar data files to simulate a few 
different real world scenarios and test any interface in development. In order 
to connect to the actual hardware, a standard "navigation" installation of 
SonaSoft is required.

If you have any questions, suggestions, or recommendations, please feel free to email: service@farsounder.com.

## Higher level clients / other interfaces
- C++ API client (WIP): [Minimal example](https://github.com/farsounder/farsounder-cpp-client-example) and [actual client](https://github.com/farsounder/farsounder-cpp-client)
- Python API client (WIP): https://github.com/farsounder/farsounder-python-client
- The Technische Universität Berlin has created a ROS package for interfacing with SonaSoft™ via the ZeroMQ/Protobuf API. You can find it [here](https://git.tu-berlin.de/farsounder_directories).


## API comparison

### Why use one over the other?
The JSON option was added by popular request, mainly regarding higher
integration complexity of the zeromq/protobuf API (despite more efficient
message size). It will be expanded in an upcoming release to expose more
functionality, while the protobuf API will only receive maintainence updates.

| Category | Protobuf / ZMQ | JSON over HTTP "REST" API |
| --- | --- | --- |
| Transport | ZeroMQ sockets | HTTP |
| Data format | Protocol Buffers | JSON |
| Performance | Faster, more efficient | Simpler, less efficient |
| Integration complexity | Higher (additional deps) | Lower (web-friendly) |
| Docs location | [IDD](https://static1.squarespace.com/static/60cce3169290423b889a1b09/t/669e7fb7eb50b43d4e2cfe2e/1721663415836/F33583-FarSounder_IDD_4.1.1.pdf) and the [nav_api.proto header](zmq_protobuf_api/cpp_client_example/windows/SDKMessageExample/proto-files/nav_api.proto) | Live docs at `http://localhost:3000/docs` (when you launch demo or static [here](https://farsounder.github.io/SDK-Integration-Examples/)) |
| Access history data | ❌ | ✅ |
| Access target data | ✅  | ✅ |
| Change processor Settings | ✅ | ✅ |
