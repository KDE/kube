project(kube-accounts-gmail)

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
