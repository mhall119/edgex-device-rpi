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
By default, the configuration and profile file used by the service are available in `res` folder, but you can override
that by passing in the `./examples/<ExampleProject>` path you want to use. See the [Example Projects](#example-projects) below for more.



