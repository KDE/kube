project(kube-accounts-kolabnow)

set(SRCS
    kolabnowsettings.cpp
    kolabnowaccountplugin.cpp
)

add_library(kolabnowaccountplugin SHARED ${SRCS})
target_link_libraries(kolabnowaccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

install(TARGETS kolabnowaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/kolabnow)
install(FILES metadata.json DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/kolabnow)
install_qml_account(kolabnow)
