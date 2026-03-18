# Python SDK Examples

This directory holds Python examples that use the higher-level `farsounder`
package instead of managing ZeroMQ and protobuf directly.

## Examples

- `simple_example/` uses `uv` and demonstrates one request/reply call plus one
  pub/sub subscription flow.
- `drawing_example/` is a larger visualization example that subscribes to live
  data and renders it with `rerun`.

The Python SDK repo is
[farsounder-python-client](https://github.com/farsounder/farsounder-python-client).
