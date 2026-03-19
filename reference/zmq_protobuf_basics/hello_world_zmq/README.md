# Hello World ZeroMQ

This is the smallest possible ZeroMQ request/reply smoke test in the repo.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Run

Start the receiver in one terminal:

```bash
./build/Release/zmq_receiver.exe # windows
./build/zmq_receiver # linux
```

Then start the sender in another:

```bash
./build/Release/zmq_sender.exe # windows
./build/zmq_sender # linux
```

This example is based on the zmq docs: https://zeromq.org/get-started/?language=cpp&library=cppzmq#