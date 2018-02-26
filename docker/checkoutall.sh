#!/bin/bash

SRCDIR="~/src"

cd $SRCDIR

if [ -d flatbuffers ]; then
    git clone https://github.com/google/flatbuffers.git
fi
cd flatbuffers
git pull
git checkout v1.6.0
cd ..

if [ -d kasync ]; then
    git clone kde:kasync
fi
cd kasync
git pull
cd ..

if [ -d kimap2 ]; then
    git clone kde:kimap2
fi
cd kimap2
git pull
cd ..

if [ -d kdav2 ]; then
    git clone kde:kdav2
fi
cd kdav2
git pull
cd ..

if [ -d sink ]; then
    git clone kde:sink
fi
cd sink
git checkout develop
git pull
cd ..

if [ -d kube ]; then
    git clone kde:kube
fi
cd kube
git checkout develop
git pull
cd ..
