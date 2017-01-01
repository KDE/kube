set(SRCS
    actionplugin.cpp
    action.cpp
    actionhandler.cpp
    actionbroker.cpp
    actionresult.cpp
    context.cpp
)

add_library(actionplugin SHARED ${SRCS})

target_link_libraries(actionplugin KF5::Async sink)
qt5_use_modules(actionplugin Core Quick Qml)

install(TARGETS actionplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/actions)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/actions)

add_subdirectory(tests)
