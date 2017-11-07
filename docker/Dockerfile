FROM fedora:27

RUN dnf -y update

RUN dnf -y install gcc-c++ git doxygen cmake bzr vim tar ninja-build clang

#kdesrcbuild deps
RUN dnf -y install sudo perl-libwww-perl perl-JSON perl-JSON-PP perl-XML-Parser perl-IPC-Cmd libcxx-devel

#Sink deps
RUN dnf -y install extra-cmake-modules qt5-qtbase-devel libgit2-devel lmdb-devel readline-devel libcurl-devel

#Test and debugging
RUN dnf -y install gdb xterm perf valgrind

#kf5-mime
RUN dnf -y install cyrus-sasl-devel

#Stuff for kube
RUN dnf -y install qt5-qtquickcontrols qt5-qtquickcontrols2-devel qt5-qtwebengine-devel qt5-qtxmlpatterns-devel
RUN dnf -y install kf5-kpackage-devel kf5-ki18n-devel kf5-kcodecs-devel kf5-kcontacts-devel kf5-kmime-devel qgpgme-devel

# Support
RUN dnf -y install icecream

# Headless UI tests
RUN dnf -y install xorg-x11-server-Xvfb

# Imap tests
RUN dnf -y install cyrus-imapd cyrus-sasl cyrus-sasl-plain gnupg2-smime

#KCachegrind needs dbus =(
RUN dnf -y install kcachegrind dbus-x11

RUN useradd -d /home/developer -m developer

# setup developer account
RUN echo 'developer ALL=NOPASSWD: ALL' >> /etc/sudoers
USER developer
ENV HOME /home/developer
WORKDIR /home/developer/

#Setup cyrus imap
# RUN sudo usermod -p `perl -e "print crypt("admin","Q4")"` cyrus
# RUN sudo useradd -p `perl -e "print crypt("doe","Q4")"` doe
# RUN sudo saslauthd -a pam && sudo /usr/libexec/cyrus-imapd/master -d && sleep 1 && sudo echo "cm user.doe" | cyradm --auth PLAIN -u cyrus -w admin localhost

#DBus For KCacheGrind
RUN sudo dbus-uuidgen --ensure

ENV QT_PLUGIN_PATH /install/lib64/plugins/
ENV LD_LIBRARY_PATH /install/lib64
ENV PATH /install/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ENV QML_IMPORT_PATH /install/lib64/qml
ENV QML2_IMPORT_PATH /install/lib64/qml
ENV XDG_CONFIG_DIRS /install/share/config
ENV XDG_DATA_DIRS /install/share/:/usr/share
RUN mkdir /tmp/runtime-developer
ENV XDG_RUNTIME_DIR /tmp/runtime-developer

ENTRYPOINT ["/bin/bash"]
