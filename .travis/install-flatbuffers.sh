#!/bin/bash
set -xeuo pipefail
wget "https://github.com/google/flatbuffers/archive/v1.8.0.tar.gz" -O /tmp/flatbuffers.tar.gz
cd /tmp
tar xvfa /tmp/flatbuffers.tar.gz
cd flatbuffers-1.8.0
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DFLATBUFFERS_BUILD_SHAREDLIB=ON
ninja
sudo ninja install
sudo cp flatc /usr/bin
cd
rm -rf /tmp/flatbuffers.tar.gz /tmp/flatbuffers-1.8.0
