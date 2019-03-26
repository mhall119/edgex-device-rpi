# Motion Detector Example

## About
Example of a motion detector using a PIR sensor, LEDs and Node-Red

## Supported Boards:
Raspberry PI 3+ - ARM64 bit

## Configuration

![Wiring Diagram](wiring.png)

## Running

```
docker-compose -f ./examples/MotionDetector/docker-compose.yaml up -d

./examples/MotionDetector/register_mqtt_export.sh

device-rpi -c ./examples/MotionDetector
```














