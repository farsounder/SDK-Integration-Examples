# C++ SDK Examples

This directory holds C++ examples that use the higher-level `farsounder`
C++ client library instead of managing ZeroMQ and protobuf directly.

## Examples

- `simple_example/` fetches the packaged SDK with CMake and demonstrates one
  request/reply call plus one pub/sub subscription flow.
- `imgui_viewer_example/` builds a Dear ImGui desktop application that renders
  `TargetData` bottom and target bins in a live 3D point-cloud view.

The canonical SDK repo is
[farsounder-cpp-client](https://github.com/farsounder/farsounder-cpp-client).
