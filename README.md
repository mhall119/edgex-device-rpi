# Device RaspberryPi GPIO Service

## About
The EdgeX Device RaspberryPi Service is developed to control/communicate with sensors connected the GPIO pins on a RaspberryPi

## Supported Boards:
Raspberry PI 3+ - ARM64 bit

## Dependencies:
The Device RaspberryPi service is developed using libmraa - a low level library that communicates with the Raspberry PI board.
The repository can be found on git at [libmraa](https://github.com/intel-iot-devkit/mraa). 

You will also need to isntall the following:
```
sudo apt install git cmake curl g++
```

## Build Instruction:

1. Check out edgex-device-rpi available at [edgex-device-rpi](https://github.com/mhall119/edgex-device-rpi)

2. Build the executable using following command
```
sh> cd edgex-device-rpi
sh> sudo ./scripts/build_deps.sh
```
This command shall build the dependencies - libmraa and device-c-sdk library to build edgex-device-rpi executable.

3. Build the executable using following command
```
sh> ./scripts/build.sh
```
This command shall build the edgex-device-rpi release image in ./build/release/device-rpi.
You can then run the device service using one of the example configurations in the ./examples/ folder with:
```
sh> ./build/release/device-rpi --confdir ./examples/__ExampleToRun__
```

## Build Docker image:

1. Check out edgex-device-rpi available at [edgex-device-rpi](https://github.com/mhall119/edgex-device-rpi)

2. Build a docker image by using the following command
```
sh> cd edgex-device-rpi
sh> docker build . -t < edgex-device-rpi > -f ./scripts/Dockerfile.alpine-3.8

```
This command shall build the dependencies - libmraa and device-c-sdk library to build edgex-device-rpi release image.
By default, the configuration and profile file used by the service are available in __'res'__ folder.















