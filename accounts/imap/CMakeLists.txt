project(kube-accounts-imap)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml)

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

kpackage_install_package(package org.kube.accounts.imap "genericqml")

install(TARGETS imapaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/imap)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/imap)
