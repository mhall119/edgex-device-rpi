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
sudo apt install git cmake curl g++ libcurl4-openssl-dev libmicrohttpd-dev libyaml-dev
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

## Build Docker image:

1. Check out edgex-device-rpi repo:
```
git clone https://github.com/mhall119/edgex-device-rpi
```

2. Install docker tools from upstream (see [instructions here](https://docs.docker.com/install/linux/docker-ce/debian/))
  
    You may also need to add your user to the `docker` group:
    ```
    sudo usermod -a -G docker $USER
    newgrp docker
    ```

3. Build a docker image by using the following command
```
cd edgex-device-rpi
docker build . -t edgex-device-rpi -f ./scripts/Dockerfile.alpine-3.8

```
This command will build the edgex-device-rpi release image.
By default, the configuration and profile file used by the service are available in __'res'__ folder, but you can override
that by passing in the `./examples/<ExampleProject>` path you want to use. See the [Example Projects](#example-projects) below for more.


## Example Projects

The EdgeX RaspberryPi Device Service comes with some pre-defined examples you can use to get started.
These projects include instructions for running EdgeX services on your PC, running the RPi service on your device, wiring diagrams for connecting sensors, and configuration files for controlling everything. 

Available exampless are:
 - [LED Control](examples/Blink/README.md)
 - [Motion Detector](examples/MotionDetector/README.md)













