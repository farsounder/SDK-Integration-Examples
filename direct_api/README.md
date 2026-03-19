# Direct API Examples

These examples connect to SonaSoft without the higher-level SDK clients.

Choose a path:
- `zmq_protobuf/cpp/` for a cross-platform C++ request/reply plus pub/sub example
- `zmq_protobuf/python/` for a Python request/reply plus pub/sub example

If you are troubleshooting your environment first, start in
`../reference/zmq_protobuf_basics/`.

# Running the examples on WSL
If you are running the examples on WSL, you'll need to pass the IP you for the host 
Windows computer running the SonaSoft demo instead of `localhost` etc. You can get
it with:
``` bash
ip route | awk '/default/ {print $3}'
```

More info at: https://learn.microsoft.com/en-us/windows/wsl/networking#identify-ip-address