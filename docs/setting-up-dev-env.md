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
        - libcurl-dev(el)
        - flatbuffers-dev
        - [KAsync](git://anongit.kde.org/kasync)
        - [KMime](git://anongit.kde.org/kmime)
        - [KCalCore](git://anongit.kde.org/kcalcore)
        - [KDAV2](git://anongit.kde.org/kdav2)
        - [KIMAP2](git://anongit.kde.org/kimap2)
    - Install Sink
- Install the Kube project:
    - `mkdir build && cd build && cmake .. && make && sudo make install`

If you did not install sink in the `/usr` directory, you might need to set the
`QT_PLUGIN_PATH` variable to something like this:
`$SINK_INSTALL_PREFIX/lib/plugins`.

In the same way, if did not install Kube in the `/usr` directory, you might
need to set the `QML2_IMPORT_PATH` to something like this:
`$KUBE_INSTALL_PREFIX/lib/qml/`.

### Flatbuffers

If your distro has a package, simply installing it should do the trick.

If it doesn't (yet), follow the instructions here: http://google.github.io/flatbuffers/flatbuffers_guide_building.html

Sink will require the flatc executable to generate some relevant headers that we require.

## Docker
Building kube in a docker containers ensures reproducability and decouples the development environment from the host system (so upgrading your host system doesn't break all your builds). To avoid having to develop inside the container directly, source, build and install directories reside on the host system.

- Go to the `docker` directory
- In the `run.sh` script, set the SOURCEDIR, BUILDDIR and INSTALLDIR variables
  to an existing path containing respectively the source, build and
  installation directory of kube. The build and installation directory should be
  empty at first, for the source directory you may use an existing directory containing the necessary source directories.
- Run the `./build.sh` script
- Run the `./run.sh` script
- If you don't have the sources available yet, run the `~/initrepositories.sh` script
- To configure the build directories, for each repository:
    - `mkdir /build/$REPO && cd /build/$REPO`
    - `cmake -DCMAKE_INSTALL_PREFIX=/install /src/$REPO`
    - `make && make install`
- You can now edit the sources outside the container, and build and run kube inside the container.
