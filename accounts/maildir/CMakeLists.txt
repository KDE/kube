project(kube-accounts-maildir)

cmake_minimum_required(VERSION 2.8.12)

include(CPack)
include(FeatureSummary)
find_package(PkgConfig)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)

set (QT_MIN_VERSION "5.4.0")
find_package(Sink CONFIG REQUIRED)
find_package(KF5Async CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

set(SRCS
    maildirsettings.cpp
    maildiraccountplugin.cpp
)

add_library(maildiraccountplugin SHARED ${SRCS})
qt5_use_modules(maildiraccountplugin Core Quick Qml)
target_link_libraries(maildiraccountplugin sink)

kpackage_install_package(package org.kube.accounts.maildir "genericqml")

install(TARGETS maildiraccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
