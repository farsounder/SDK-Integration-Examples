# ZeroMQ/Protobuf Basics

These examples are for troubleshooting and setup validation, not for primary
SonaSoft integration.

Use them when you want to isolate one dependency at a time:

- `hello_world_zmq/` checks a basic ZeroMQ request/reply round trip
- `protobuf_only/` checks protobuf generation plus simple read/write code

If both of these work but the direct SonaSoft examples do not, the problem is
more likely to be host, port, firewall, or API-version related rather than a
basic dependency issue.
