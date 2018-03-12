#!/bin/bash
set -xeuo pipefail
IFS=$'\n\t'

package=$1
version=$2

git clone --branch="$version" --depth=1 --recursive "https://anongit.kde.org/$package" "/tmp/$package"
cd "/tmp/$package"
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr
ninja && sudo ninja install
cd
rm -rf "/tmp/$package"
