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

install(TARGETS maildiraccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
install(FILES metadata.json DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/maildir)
install_qml_account(maildir)
