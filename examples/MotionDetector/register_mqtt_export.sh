#1/bin/sh

curl -X POST -d '{
    "name":"ExampleRPiMotionEvents",
    "addressable":{
        "name":"EdgeXMQTTBroker",
        "protocol":"tcp",
        "address":"edgex_mqtt_broker",
        "port":1883,
        "publisher":"EdgeXExportPublisher",
        "topic":"ExampleRpiMotion"
    },
    "format":"JSON",
    "filter":{
        "deviceIdentifiers":["RPiMotionDetector"]
    },
    "enable":true,
    "destination":"MQTT_TOPIC"
}' http://localhost:48071/api/v1/registration
