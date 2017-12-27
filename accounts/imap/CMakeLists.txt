project(kube-accounts-imap)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)

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

install(TARGETS imapaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/imap)
install_qml_account(imap)
