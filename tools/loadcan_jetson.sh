#!/bin/bash

# Information on this process is found in the Jetson AGX Orin Developer Guide

# Enable CAN on Jetson
busybox devmem 0x0c303018 w 0x458 # CAN0 DIN (29)
busybox devmem 0x0c303010 w 0x400 # CAN0 DOUT (31)
busybox devmem 0x0c303008 w 0x458 # CAN1 DIN (37)
busybox devmem 0x0c303000 w 0x400 # CAN1 DOUT (33)

# Load necessary kernel drivers
modprobe can
modprobe can_raw
modprobe mttcan

# Set CAN interface properties
ip link set can0 up type can bitrate 1000000
ip link set can1 up type can bitrate 1000000

# Change buffer queue length
ifconfig can0 txqueuelen 250
ifconfig can1 txqueuelen 250
