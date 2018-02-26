# Setting up the development environment

Multiple choices are available for developing the Kube project:

- Manually
- Docker

## Manually

- If sink is not in you repositories:
    - Install the dependencies of Sink:
        - extra-cmake-modules
        - qt5-qtbase-devel / qtbase5-dev
        - libgit2-dev(el)
        - lmdb-devel / liblmdb-dev
        - readline-devel / libreadline-dev
        - libcurl-dev(el)
        - kf5-kimap
        - kf5-kimap-devel
        - [KAsync](https://github.com/KDE/kasync/releases)
    - Install Sink
- Install the Kube project:
    - `mkdir build && cd build && cmake .. && make && sudo make install`

If you did not install sink in the `/usr` directory, you might need to set the
`QT_PLUGIN_PATH` variable to something like this:
`$SINK_INSTALL_PREFIX/lib/plugins`.

In the same way, if did not install Kube in the `/usr` directory, you might
need to set the `QML2_IMPORT_PATH` to something like this:
`$KUBE_INSTALL_PREFIX/lib/qml/`.

## Docker

- Go to the `docker` directory
- In the `run.sh` script, set the SOURCEDIR, BUILDDIR and INSTALLDIR variables
  to an existing path containing respectively the source, build and
  installation directory of kube. The build and installation directory can be
  empty at first.
- Run the `./build.sh` script
- Run the `./run.sh` script
- In the now opened container shell, run `cmake -DCMAKE_INSTALL_PREFIX=/install /src`
- Run `make install`
