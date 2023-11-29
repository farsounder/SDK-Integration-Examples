# Minimal SDK Example

## Two methods of communicating with SonaSoft™
There are two methods available to communicate with SonaSoft™. The first uses
messages that are serialized using Google's Protocol Buffers sent over ZeroMQ
sockets. The second uses an HTTP REST API that sends and receives JSON.

## Option 1: Protobuf/ZMQ SonaSoft™ SDK message passing
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

## Option 2: HTTP REST API
The second option for integration is to use the HTTP REST API. This is a simple
API server that runs on the machine that SonaSoft™ is installed on. Sending
JSON over HTTP is much simpler than using ZeroMQ and Protobuf, but it is not
as fast or efficient. Live docs for the API are available on the machine that
SonaSoft™ is installed on, you can find them by starting SonaSoft and
going to http://localhost:3000/docs in your browser.

## Playing back recorded data w/ SonaSoft™ SDK
The FarSounderSDK package has an "SDK Version" of SonaSoft™ that can be used to
run through recorded sonar data files to simulate a few different real world
scenarios and test any interface in development.
The FarSounderSDK package (can be obtained by contacting us if you do not have 
it already) contains an "SDK Version" of SonaSoft
that can be used to run through recorded sonar data files to simulate a few 
different real world scenarios and test any interface in development. In order 
to connect to the actual hardware, a standard "navigation" installation of 
SonaSoft is required.

If you have any questions, suggestions, or recommendations, please feel free to email: service@farsounder.com.