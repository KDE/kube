#!/bin/bash

#This script helps testing the scenario where two kube instance are running in separate namespaces. This is what happens with flatpak, and it currently breaks lmdb.

sudo unshare -fp env PATH=/install/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin QT_PLUGIN_PATH=/install/lib64/plugins/ QML2_IMPORT_PATH=/install/lib64/qml XDG_DATA_DIRS=/install/share/ LD_LIBRARY_PATH=/install/lib64 XDG_CONFIG_DIRS=/install/share/config /install/bin/kube
