#!/bin/sh
set -e -x

# Dependencies
if [ ! -d deps ]
then
  basedir=`pwd`

  mkdir deps

  cd $basedir/deps

  git clone https://github.com/intel-iot-devkit/mraa.git
  cd mraa
  # This version contain fix to identify raspberry-pi 3
  git reset --hard d320776

# patch for raspberryPI
  patch -p1 < $basedir/scripts/rpi_patch

  mkdir -p build && cd build

# always install in lib folder
  cmake -DBUILDSWIG=OFF -DCMAKE_INSTALL_LIBDIR=lib ../.
  make && make install

# get c-sdk from edgexfoundry
  cd $basedir/deps
  wget https://github.com/edgexfoundry/device-sdk-c/archive/0.7.1.tar.gz
  tar -xzf 0.7.1.tar.gz
  cd device-sdk-c-0.7.1
  ./scripts/build.sh
  cp -rf include/* /usr/include/
  cp build/release/c/libcsdk.so /usr/lib/

  rm -rf $basesdir/deps
fi
