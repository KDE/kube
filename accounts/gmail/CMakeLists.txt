project(kube-accounts-gmail)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

set(SRCS
    gmailsettings.cpp
    gmailaccountplugin.cpp
)

add_library(gmailaccountplugin SHARED ${SRCS})
target_link_libraries(gmailaccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

kpackage_install_package(package org.kube.accounts.gmail "genericqml")

install(TARGETS gmailaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/gmail)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/gmail)
