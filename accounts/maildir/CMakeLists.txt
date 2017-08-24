project(kube-accounts-maildir)

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

kpackage_install_package(package org.kube.accounts.maildir "genericqml")

install(TARGETS maildiraccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
