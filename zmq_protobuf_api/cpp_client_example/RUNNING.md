## Running the example

### Windows
This version of the example uses C++ in Visual Studio 2022 on Windows. The
required versions of the zeromq and protobuf libraries are included in the
project. Before running, you will need to compile the .proto files into cpp
files. This can be done by running the build_protos.bat file in the
SDKMessageExample folder.

### Ubuntu
This version runs was tested using Ubuntu on WSL2, detailed instructions for
setting up are in the [ubuntu folder](/zmq_protobuf_api/cpp_client_example/ubuntu/readme.md).

### Docker
Instructions for running a containerized version of the linux example [here](https://github.com/farsounder/SDKMessageExample/tree/master/ubuntu#running-in-docker-container)
