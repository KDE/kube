set(SRCS
    actionplugin.cpp
    action.cpp
    actionhandler.cpp
    actionbroker.cpp
    context.cpp
)

add_library(actionplugin SHARED ${SRCS})

qt5_use_modules(actionplugin Core Quick Qml)

install(TARGETS actionplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/actions)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/actions)
