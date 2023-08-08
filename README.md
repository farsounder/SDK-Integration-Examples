# Minimal SDK Example
## Example implementation - SonaSoft™ SDK message passing
This example implements both methods of commumicating with SonaSoft via the network API. There is a detailed description
in the Interface Design Definition available on the [SDK page of the website](https://www.farsounder.com/software-development-kit)
1. Publish / Subscribe
2. Request / Reply

The necessary binaries for ZeroMQ are also included. However if you would like
to download either of these dependencies to run differnt versions based on your
requirements - or to look at their official docs:
* ZeroMQ - https://zeromq.org/
* ProtocolBuffers - https://developers.google.com/protocol-buffers

We use ProtocolBuffers to manage structured data in a language neutral way, and ZeroMQ to pass serialized "proto" messages between processes.

The FarSounderSDK package (can be obtained by contacting us if you do not have already) contains an "SDK Version" of SonaSoft
that can be used to run through recorded sonar data files to simulate a few different real world scenarios and test any
interface in development. In order to connect to the actual hardware, a standard "navigation" installation of SonaSoft is
required.

Here's a schematic of those two cases:

![sdk_setup](https://github.com/farsounder/SDKMessageExample/assets/5819478/745e7ef9-8b12-402a-bdef-15f510fcee4e)


If you have any questions, suggestions, or recommendations, please feel free to email: service@farsounder.com.

## Running the example

### Windows
This version of the example uses C++ in Visual Studio 2022 on Windows. The
required versions of the zeromq and protobuf libraries are included in the
project. Before running, you will need to compile the .proto files into cpp
files. This can be done by running the build_protos.bat file in the
SDKMessageExample folder.

### Ubuntu
This version runs was tested using Ubuntu on WSL2, detailed instructions for
setting up are in the [ubuntu folder](/ubuntu/readme.md).

### Docker
Instructions for running a containerized version of the linux example [here](https://github.com/farsounder/SDKMessageExample/tree/master/ubuntu#running-in-docker-container)
