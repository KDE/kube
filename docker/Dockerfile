FROM fedora:27

RUN dnf -y update

RUN dnf -y install gcc-c++ git doxygen cmake bzr vim tar ninja-build clang procps-ng libcxx-devel

#Sink deps
RUN dnf -y install extra-cmake-modules qt5-qtbase-devel libgit2-devel lmdb-devel readline-devel libcurl-devel

#Test and debugging
RUN dnf -y install gdb xterm perf valgrind strace kcachegrind dbus-x11

#kf5-mime
RUN dnf -y install cyrus-sasl-devel

#Stuff for kube
RUN dnf -y install qt5-qtquickcontrols qt5-qtquickcontrols2-devel qt5-qtwebengine-devel qt5-qtxmlpatterns-devel
RUN dnf -y install kf5-kpackage-devel kf5-ki18n-devel kf5-kcodecs-devel kf5-kcontacts-devel kf5-kmime-devel qgpgme-devel

# Imap tests
RUN dnf -y install cyrus-imapd cyrus-sasl cyrus-sasl-plain gnupg2-smime

#Fonts
RUN dnf -y install google-noto-serif-fonts google-noto-sans-fonts

#Required for the symbolizer for asan
RUN dnf -y install llvm

RUN dnf -y install sudo
RUN dnf -y install pinentry-gtk
RUN dnf -y install gammaray heaptrack
RUN dnf -y install xapian-core-devel

RUN useradd -d /home/developer -m developer

#Setup cyrus imap
RUN echo 'altnamespace: 0' >> /etc/imapd.conf
RUN echo 'unixhierarchysep: 0' >> /etc/imapd.conf
RUN usermod -p `perl -e "print crypt("admin","Q4")"` cyrus
RUN useradd -p `perl -e "print crypt("doe","Q4")"` doe
RUN /usr/bin/sscg --package cyrus-imapd --cert-file /etc/pki/cyrus-imapd/cyrus-imapd.pem --cert-key-file /etc/pki/cyrus-imapd/cyrus-imapd-key.pem --ca-file /etc/pki/cyrus-imapd/cyrus-imapd-ca.pem
RUN saslauthd -a pam && /usr/libexec/cyrus-imapd/master -d && sleep 1 && echo "cm user.doe" | cyradm --auth PLAIN -u cyrus -w admin localhost

#DBus For KCacheGrind
RUN dbus-uuidgen --ensure

# setup developer account
RUN echo 'developer ALL=NOPASSWD: ALL' >> /etc/sudoers
USER developer
ENV HOME /home/developer
WORKDIR /home/developer/

ENV QT_PLUGIN_PATH /install/lib64/plugins/
ENV LD_LIBRARY_PATH /install/lib64
ENV PATH /install/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ENV QML_IMPORT_PATH /install/lib64/qml
ENV QML2_IMPORT_PATH /install/lib64/qml
ENV XDG_CONFIG_DIRS /install/share/config
ENV XDG_DATA_DIRS /install/share/:/usr/share
RUN mkdir /tmp/runtime-developer
ENV XDG_RUNTIME_DIR /tmp/runtime-developer
ENV LANG en_US.UTF-8

ADD bashrc /home/developer/.bashrc
ADD startimap.sh /home/developer/startimap.sh

ADD keyconfig /home/developer/keyconfig
ADD setupkolabnowtest.sh /home/developer/setupkolabnowtest.sh
ADD kubeunlocked.sh /home/developer/kubeunlocked.sh
RUN gpg2 --batch --generate-key /home/developer/keyconfig
ADD gpg-agent.conf /home/developer/.gnupg/gpg-agent.conf
ADD gpg.conf /home/developer/.gnupg/gpg.conf
