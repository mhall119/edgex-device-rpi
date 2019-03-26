#!/bin/sh
set -e -x

# Find root directory and system type

ROOT=$(dirname $(dirname $(readlink -f $0)))
echo $ROOT
cd $ROOT

# Cmake release build

mkdir -p $ROOT/build/release
cd $ROOT/build/release
cmake -DCMAKE_BUILD_TYPE=Release $ROOT/src/c
make 2>&1 | tee release.log

