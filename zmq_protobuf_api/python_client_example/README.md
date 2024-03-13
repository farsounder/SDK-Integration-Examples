# Python client example

This is a simple example of how to subscribe to a a ZeroMQ socket and receive
protobuf messages from SonaSoft™ in python. The example receives and plots a
single ping of sonar data.

## Running the example

To run the example install the dependencies (you may want to use a virtual environment):

```bash
pip install -r requirements.txt
```

Then build the python protobuf files with `build_protos_win.bat` on Windows or `build_protos_linux.sh` on Linux.

Finally you can run the example with:

```bash
python plot.single.ping.py
```
