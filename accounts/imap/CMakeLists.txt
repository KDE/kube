project(kube-accounts-imap)

cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0063 NEW)

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

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Concurrent Quick Test Gui)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

set(SRCS
    imapsettings.cpp
    imapaccountplugin.cpp
)

add_library(imapaccountplugin SHARED ${SRCS})
target_link_libraries(imapaccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

add_library(imapaccount_static STATIC ${SRCS})
target_link_libraries(imapaccount_static
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)
add_subdirectory(tests)

kpackage_install_package(package org.kube.accounts.imap "genericqml")

install(TARGETS imapaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/imap)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/imap)
