FROM fedora:33

RUN dnf -y update

RUN dnf -y install gcc-c++ git doxygen cmake bzr vim tar make ninja-build clang procps-ng libcxx-devel sudo rsyslog llvm \
    extra-cmake-modules qt5-qtbase-devel libgit2-devel lmdb-devel libcurl-devel xapian-core-devel flatbuffers-devel \
    gdb xterm perf valgrind strace kcachegrind dbus-x11 gammaray heaptrack hotspot \
    qt5-qtquickcontrols qt5-qtquickcontrols2-devel qt5-qtwebengine-devel qt5-qtxmlpatterns-devel \
    kf5-ki18n-devel kf5-kcodecs-devel kf5-kcontacts-devel kf5-kmime-devel gpgme-devel kf5-kcalendarcore-devel \
    cyrus-imapd cyrus-sasl cyrus-sasl-devel cyrus-sasl-plain gnupg2-smime pinentry-gtk \
    google-noto-serif-fonts google-noto-sans-fonts \
    xorg-x11-server-Xvfb qt5ct

RUN useradd -d /home/developer -m developer

ADD rsyslog.conf /etc/rsyslog.conf
#Setup cyrus imap
ADD imapd.conf /etc/imapd.conf
RUN usermod -p `perl -e "print crypt("admin","Q4")"` cyrus
RUN useradd -p `perl -e "print crypt("doe","Q4")"` doe
RUN /usr/bin/sscg --package cyrus-imapd --cert-file /etc/pki/cyrus-imapd/cyrus-imapd.pem --cert-key-file /etc/pki/cyrus-imapd/cyrus-imapd-key.pem --ca-file /etc/pki/cyrus-imapd/cyrus-imapd-ca.pem
RUN chmod 644 /etc/pki/cyrus-imapd/cyrus-imapd-key.pem
# Because the following tends to fail we create the user in the test setup script now.
# RUN saslauthd -a shadow && /usr/libexec/cyrus-imapd/master -d && sleep 1 && echo "cm user.doe" | cyradm --auth PLAIN -u cyrus -w admin localhost

#Qt 5.12 js Date doesn't play well with the default UTC timezone
RUN rm /etc/localtime; ln -s /usr/share/zoneinfo/Europe/Zurich /etc/localtime

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
ENV QT_QPA_PLATFORMTHEME qt5ct

RUN git config --global url."git://anongit.kde.org/".insteadOf kde: && \
    git config --global url."ssh://git@git.kde.org/".pushInsteadOf kde:

ADD bashrc /home/developer/.bashrc
ADD startimap.sh /home/developer/startimap.sh


ADD setupkolabnowtest.sh /home/developer/setupkolabnowtest.sh
ADD setupperformancetest.sh /home/developer/setupperformancetest.sh
ADD setupgoogletest.sh /home/developer/setupgoogletest.sh
ADD kubeunlocked.sh /home/developer/kubeunlocked.sh
ADD initrepositories.sh /home/developer/initrepositories.sh

ADD keyconfig /home/developer/keyconfig
ADD gpg-agent.conf /home/developer/.gnupg/gpg-agent.conf
ADD gpg.conf /home/developer/.gnupg/gpg.conf
ADD gdbinit /home/developer/.gdbinit
ADD enableDebug.sh /home/developer/enableDebug.sh
ADD private-key /home/developer/private-key
ADD public-key /home/developer/public-key
ADD qt5ct /home/developer/.config/qt5ct
RUN sudo chown developer:developer /home/developer/*
RUN sudo chown developer:developer /home/developer/.* -R

RUN gpg2 --import /home/developer/public-key
RUN gpg2 --batch --import /home/developer/private-key
#Better qt support
RUN git clone https://github.com/Lekensteyn/qt5printers.git ~/.gdb/qt5printers/
