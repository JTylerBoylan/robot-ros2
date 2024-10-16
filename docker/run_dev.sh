#!/bin/bash

PROJECT_NAME=robot-ros2
PROJECT_DIR=/home/ubuntu/ros2_ws/src/

# Get the absolute path to the directory containing this script
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# Allow root to connect to the X server
if [ -n "$DISPLAY" ]; then
    xhost +local:root > /dev/null
fi

# Start the Docker container
docker run -it \
    --rm \
    --net host \
    --privileged \
    --name ${PROJECT_NAME} \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v "/${SCRIPT_DIR}/../:${PROJECT_DIR}" \
    -w "${PROJECT_DIR}/../" \
    ${PROJECT_NAME}:latest \
    bash
