set(accountsplugin_SRCS
    accountsplugin.cpp
    accountfactory.cpp
    accountsmodel.cpp
    maildircontroller.cpp
)

add_library(accountsplugin SHARED ${accountsplugin_SRCS})

qt5_use_modules(accountsplugin Core Quick Qml)
target_link_libraries(accountsplugin sink mailplugin)

install(TARGETS accountsplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/accounts)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/accounts)
