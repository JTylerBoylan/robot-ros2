# Jetson AGX Orin Setup

## Hardware Info

* NVIDIA Jetson AGX Orin 64GB Developer Kit
* SAMSUNG 990 PRO with Heatsink - PCIe 4.0 NVMe M.2 SSD

## Flashing

### Resources
* [Download NVIDIA SDK Manager](https://developer.nvidia.com/sdk-manager)
* [Install Jetson Software with SDK Manager](https://docs.nvidia.com/sdk-manager/install-with-sdkm-jetson/index.html)

### Procedure

1. Open SDK Manager on Host Machine (Ubuntu 22.04 x86_64)
2. Power Jetson in Recovery Mode
   - Press the Power button (left) and press the Reset button (right), all while holding down the Recovery button (middle)
3. Connect Jetson to Host Machine by a USB-C cable
4. Select Jetson AGX Orin 64GB Board in SDK Manager
5. Select to install all Jetson SDK components
6. Wait for Download and Install to finish
   - Some configuration prompts will appear during the installation
   ```
   Username: selqie
   Password: *Lab password*
   ```
7. When complete, attach monitor to Jetson and perform software installation steps

## Install Software

### 1. Update and Upgrade
```
sudo apt update && sudo apt upgrade
```

### 2. Add SSH Key
1. [Generate new SSH Key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent)
2. [Add SSH Key to Github](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account)

### 3. Install SELQIE Repository
```
```

### 3. Run Installation Bash Script
- Install System Dependencies
- Install ROS Humble
- Install ROS Dependencies
- Install MuJoCo
- Install OSQP
- Install SBMPO
- Setup CAN Boot Service
- Setup IMU Microstrain Rules
- Setup GPIO Configuration
- Reboot