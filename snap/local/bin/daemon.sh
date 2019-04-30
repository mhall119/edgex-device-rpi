#!/bin/bash -e

# get the config item as a bash list (from a json list)
mapfile -t array < <(snapctl get -d daemon-args | jq -r '."daemon-args" | .[]')

# run the binary with the arguments
"$SNAP/bin/device-rpi" "${array[@]}"
