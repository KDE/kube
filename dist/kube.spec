Name:           kube
Version:        0.3.0
Release:	    0%{?dist}
Summary:        kube

Group:          Applications/Desktop
License:        GPL
URL:            https://docs.kolab.org/about/kube
Source0:        kube-%{version}.tar.xz

BuildRequires:  boost-devel
BuildRequires:  cmake >= 2.8.12
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  kasync-devel
BuildRequires:  kf5-kpackage-devel
BuildRequires:  kf5-kcodecs-devel
BuildRequires:  kf5-kmime-devel
BuildRequires:  kf5-kcontacts-devel
BuildRequires:  kf5-ki18n-devel
BuildRequires:  lmdb-devel
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qtwebengine-devel
BuildRequires:  sink-devel
BuildRequires:  libcurl-devel
BuildRequires:  qt5-qtquickcontrols2-devel
BuildRequires:  qgpgme-devel
BuildRequires:  gpgme >= 1.8.0

Requires:       qt5-qtquick1
Requires:       qt5-qtquickcontrols
Requires:       qt5-qtquickcontrols2
Requires:       sink >= 0.3.0
Requires:       google-noto-sans-fonts

%description
kube.kde.org

%prep
%setup -q

%build
mkdir -p build/
pushd build
%{cmake} \
    -DQML_INSTALL_DIR:PATH=%{_libdir}/qt5/qml/ \
    ..

make %{?_smp_mflags}
popd

%install
pushd build
%make_install
popd

%files
%doc
%{_bindir}/kube
%dir %{_libdir}/qt5/
%dir %{_libdir}/qt5/qml/
%dir %{_libdir}/qt5/qml/org/
%{_libdir}/qt5/qml/org/kube/
%{_libdir}/libkube_otp.so
%{_datadir}/appdata/org.kde.kube.appdata.xml
%{_datadir}/applications/org.kde.kube.desktop
%{_datadir}/icons/kube
%{_datadir}/icons/hicolor/256x256/apps/kube_icon.png
%{_datadir}/icons/hicolor/scalable/apps/kube_icon.svg
%{_datadir}/icons/hicolor/scalable/apps/kube_logo.svg
%{_datadir}/icons/hicolor/scalable/apps/kube_symbol.svg
%{_datadir}/kpackage/genericqml/org.kube.*/
%{_datadir}/metainfo/org.kube.*

%changelog
