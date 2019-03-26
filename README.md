# Device RaspberryPi GPIO Service

## About
The EdgeX Device RaspberryPi Service is developed to control/communicate with sensors connected the GPIO pins on a RaspberryPi

## Supported Boards:
Raspberry PI 3+ - ARM64 bit

## Dependencies:
The Device RaspberryPi service is developed using libmraa - a low level library that communicates with the Raspberry PI board.
The repository can be found on git at [libmraa](https://github.com/intel-iot-devkit/mraa). 

## Build Instruction:

1. Check out edgex-device-rpi available at [device-grove-c](https://github.com/mhall119/edgex-device-rpi)

2. Build a docker image by using the following command
```
sh> cd edgex-device-rpi
sh> docker build . -t < edgex-device-rpi > -f ./scripts/Dockerfile.alpine-3.8

```
This command shall build the dependencies - libmraa and device-c-sdk library to build edgex-device-rpi release image.
By default, the configuration and profile file used by the service are available in __'res'__ folder.

## Configuration for docker image
1. Port number specified in the configuration.toml
2. --device=/dev/ < i2c-device > to map host device to the container. For Raspberry PI, it is i2c-1.

**Note:** On Raspberry PI, make sure that i2c_arm=on is set.













