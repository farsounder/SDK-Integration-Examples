# This script demonstrates how to subscribe to the ZeroMQ publisher and
# to receive TargetData messages and then plot a single ping in python.
# It was originally contributed by: Benjamin Kiefer <benjamin@lookout.team>
import sys
import os
import zmq
import numpy as np
import matplotlib.pyplot as plt

# Get the directory of your script
dir_path = os.path.dirname(os.path.realpath(__file__))

# Add the 'python_proto' directory to the Python path
sys.path.append(os.path.join(dir_path, 'python_proto'))


# Adjust the imports based on your protobuf definitions
from python_proto import nav_api_pb2


# Connect to the ZeroMQ publisher
def connect_to_publisher(port="61502"):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(f"tcp://localhost:{port}")
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    return socket


# Parse a single Bin into a tuple
def parse_bin(b):
    return (b.cross_range, b.down_range, b.depth)


# Parse the bottom grid data
def parse_bottom_data(target_data):
    bottom_grid = [parse_bin(bin) for bin in target_data.bottom]
    return np.array(bottom_grid)


# Parse in-water target groups data
def parse_groups_data(target_data):
    groups_data = []
    for group in target_data.groups:
        group_bins = [parse_bin(bin) for bin in group.bins]
        groups_data.extend(group_bins)
    return np.array(groups_data)


# Adjusted plot function to only visualize in-water targets
def plot_data(groups_data, **kwargs):
    ax = plt.gca()
    # Only plot in-water targets if available
    if groups_data.size > 0:
        ax.scatter(
            groups_data[:, 0], groups_data[:, 1], groups_data[:, 2], **kwargs)
    ax.set_title('3D Target Data Visualization')
    ax.set_xlabel('Cross Range (m)')
    ax.set_ylabel('Down Range (m)')
    ax.set_zlabel('Depth (m)')


# Receive and parse the TargetData message
def receive_target_data(socket):
    message = socket.recv()
    target_data = nav_api_pb2.TargetData()
    target_data.ParseFromString(message)
    return target_data


# Main function to run the visualization
def main():
    # getting the data
    socket = connect_to_publisher()
    target_data = receive_target_data(socket)
    groups_data = parse_groups_data(target_data)
    bottom_data = parse_bottom_data(target_data)

    # plotting
    fig = plt.figure()
    fig.add_subplot(111, projection='3d')
    plot_data(groups_data, label="In-water Targets", marker="^", color="r")
    plot_data(bottom_data, label="Seafloor Detections", marker="o", color="b")
    plt.legend()
    plt.show()

if __name__ == "__main__":
    main()
