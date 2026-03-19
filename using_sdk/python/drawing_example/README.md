# Python SDK Drawing Example

This is a simple example that  subscribes to data from the Argos system using
the Python SDK and visualizes it. To use it, the SonaSoft SDK demo should be
running to process and send pre-recorded data, or you should be connected to
a live installation.

The only currently supported display backend is [rerun.io](https://rerun.io/),
but the structure is intended to make it easy to prototype other viewers and
inspect how to work with the received SDK data.

It's set up to show:
- a point cloud of the bottoms and targets, accumulated each ping
- a grid (1 meter but configurable - only sent every 10 pings - configurable)
- a surface mesh made from the 1m grid (sent even less as it's slow)
- a realtime surface mesh from the live bottoms (each ping)

This current version is pretty slow. It can only keep up in short runs because
sending the whole set of grids and meshes gets more expensive over time. There
is minimal edge-case and error handling because it is primarily meant as a
prototyping example to help you get started.

## Run It

Use whatever package manager you like but I recommend [uv](https://docs.astral.sh/uv/).

```
uv sync
uv run main.py # optional --log-level DEBUG|INFO|WARNING
```
It should open Rerun and, if the SonaSoft SDK demo is running, start displaying
data.

Here's an example with gridded surface, grids and live surface toggled visible in Rerun:
<img width="1896" height="1047" alt="image" src="https://github.com/user-attachments/assets/34ba3dbb-cefc-4b0c-916d-779a2d64d4e3" />
