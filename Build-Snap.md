## Build Snap package:

1. Check out edgex-device-rpi repo:
```
git clone https://github.com/mhall119/edgex-device-rpi
```

2. Install the **Snapcraft** tool from the snap store:
  
    ```
    snap install snapcraft
    ```

3. Build a snap package by using the following command
```
cd edgex-device-rpi
snapcraft snap
```

This command will build the edgex-device-rpi release image.
By default, the configuration and profile file used by the service are available in `$SNAP_DATA/res` folder, but you can override
that by passing in the `$SNAP_DATA/examples/<ExampleProject>` path you want to use. See the [Example Projects](#example-projects) below for more.



