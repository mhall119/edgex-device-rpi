# Device RaspberryPi GPIO Service

## About
The EdgeX Device RaspberryPi Service is developed to control/communicate with sensors connected the GPIO pins on a RaspberryPi

## Supported Boards:
Raspberry PI 3+ - ARM64 bit

## Dependencies:
The Device RaspberryPi service is developed using libmraa - a low level library that communicates with the Raspberry PI board.
The repository can be found on git at [libmraa](https://github.com/intel-iot-devkit/mraa). 

You will need some build tools to compile and run the Device RaspberryPi service.
On Rasbian or Ubuntu, these can be installed with:
```
sudo apt install git cmake curl g++ libcurl4-openssl-dev libmicrohttpd-dev libyaml-dev
```

## Build Instruction:

1. Check out edgex-device-rpi repo:
```
git clone https://github.com/mhall119/edgex-device-rpi
```

2. Build the executable using following command
```
cd edgex-device-rpi
sudo ./scripts/build_deps.sh
```
This command shall build the dependencies - `libmraa` and `device-c-sdk` library to build edgex-device-rpi executable.

3. Build the executable using following command
```
./scripts/build.sh
```
This command shall build the edgex-device-rpi release image in `./build/release/device-rpi`.
You can then run the device service using one of the example configurations in the ./examples/ folder with:
```
./build/release/device-rpi --confdir ./examples/<ExampleProject>
```

Be sure to read and follow the setup instructions for the example project before running `device-rpi`

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
This command shall build the dependencies - libmraa and device-c-sdk library to build edgex-device-rpi release image.
By default, the configuration and profile file used by the service are available in __'res'__ folder, but you can override
that by passing in the `./examples/<ExampleProject>` path you want to use.















