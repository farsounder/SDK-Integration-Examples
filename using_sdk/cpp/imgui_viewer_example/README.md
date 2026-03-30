# C++ SDK ImGui Viewer Example

This example uses the higher-level `farsounder` C++ client library together
with Dear ImGui, GLFW, and OpenGL to present a live 3D view of `TargetData`.

It subscribes to `TargetData`, converts bottom and target bins into a local
metric coordinate frame, and renders them as colored point clouds in a native
desktop window.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

Like `simple_example`, this downloads the packaged farsounder SDK release with
CMake. It also fetches GLFW and Dear ImGui during configure.

On Windows, the build copies `farsounder.dll` next to the example executable.

On Linux, make sure common OpenGL and X11/Wayland development packages are
available before configuring. Typical Ubuntu packages include:

```bash
sudo apt install libgl1-mesa-dev libx11-dev libxcursor-dev libxi-dev libxinerama-dev libxrandr-dev
```

## Run

```bash
./build/Release/sonasoft_sdk_cpp_imgui_viewer_example.exe # windows
./build/sonasoft_sdk_cpp_imgui_viewer_example # linux
```

You can optionally pass a different host:

```bash
./build/Release/sonasoft_sdk_cpp_imgui_viewer_example.exe 192.168.1.10
```
