# Device RaspberryPi GPIO Service

## About
The EdgeX RaspberryPi Device Service is developed to control/communicate with sensors connected the GPIO pins on a RaspberryPi

## Supported Boards:
Raspberry PI 3+
 - Ubuntu 18.04 64bit
 - Raspbian Stretch 32bit

## Dependencies:

You will also need some build tools to compile and run the Device RaspberryPi service.
On Rasbian or Ubuntu, these can be installed with:
```
sudo apt install git cmake curl g++ libcurl4-openssl-dev libmicrohttpd-dev libyaml-dev uuid-dev
```

You will also need `libcbor-dev`, which is not available in the latest stable Raspbian release. You can manually download and install it with:
```
wget http://ftp.us.debian.org/debian/pool/main/libc/libcbor/libcbor0_0.5.0+dfsg-2_armhf.deb
sudo dpkg -i libcbor0_0.5.0+dfsg-2_armhf.deb
wget http://ftp.us.debian.org/debian/pool/main/libc/libcbor/libcbor-dev_0.5.0+dfsg-2_armhf.deb
sudo dpkg -i libcbor-dev_0.5.0+dfsg-2_armhf.deb
```

## Build Instruction:

1. Check out edgex-device-rpi repo:
```
git clone https://github.com/mhall119/edgex-device-rpi
cd edgex-device-rpi
```

2. Download and build the dependencies:
```
sudo ./scripts/build_deps.sh
```
This command will download and build both the `libmraa` and `device-c-sdk` libraries needed to build the edgex-device-rpi executable.

3. Build the executable:
```
./scripts/build.sh
```

You can then run the device service using one of the example configurations in the [./examples/](./examples/) folder with:
```
./build/release/device-rpi --confdir ./examples/<ExampleProject>
```

Be sure to read and follow the setup instructions for the example project before running `device-rpi`. See the [Example Projects](#example-projects) below for more.

## Example Projects

The EdgeX RaspberryPi Device Service comes with some pre-defined examples you can use to get started.
These projects include instructions for running EdgeX services on your PC, running the RPi service on your device, wiring diagrams for connecting sensors, and configuration files for controlling everything. 

Available exampless are:
 - [LED Control](examples/Blink/README.md)
 - [Motion Detector](examples/MotionDetector/README.md)













