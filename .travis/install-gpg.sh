#!/bin/bash
set -xeuo pipefail

package=$1
version=$2
shift
shift
configure_opts=( "$@" )

wget "https://www.gnupg.org/ftp/gcrypt/$package/$package-$version.tar.bz2" -O "/tmp/$package.tar.bz2"
cd /tmp
tar xvfa "/tmp/$package.tar.bz2"
cd "$package-$version"
# Expand configure_opts only if configure_opts is not undefined
# (in older versions of Bash, empty array is considered undefined)
./configure "${configure_opts[@]+"${configure_opts[@]}"}"
make
sudo make install
cd
rm -rf "/tmp/$package.tar.bz2" "/tmp/$package-$version"
