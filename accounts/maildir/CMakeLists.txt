project(kube-accounts-maildir)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Concurrent Quick Test Gui)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

set(SRCS
    maildirsettings.cpp
    maildiraccountplugin.cpp
)

add_library(maildiraccountplugin SHARED ${SRCS})
target_link_libraries(maildiraccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

add_library(maildiraccount_static STATIC ${SRCS})
target_link_libraries(maildiraccount_static
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)
add_subdirectory(tests)

kpackage_install_package(package org.kube.accounts.maildir "genericqml")

install(TARGETS maildiraccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
