SOURCEDIR=~/kdebuild/kube/source
BUILDDIR=~/kdebuild/kube/build
INSTALLDIR=~/kdebuild/kube/install
NAME=kubedev

build:
	docker build -t ${NAME} .

rebuild:
	docker build --no-cache -t ${NAME} .

run:
	docker run --rm -ti --privileged -u developer --security-opt seccomp:unconfined -v /tmp/.docker.xauth:/tmp/.docker.xauth -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri/card0:/dev/dri/card0 -e DISPLAY=:0 -e XAUTHORITY=/tmp/.docker.xauth -v ${SOURCEDIR}:/src -v ${BUILDDIR}:/build -v ${INSTALLDIR}:/install -w /build/ ${NAME} '/bin/bash'
