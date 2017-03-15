set(SRCS
    notificationplugin.cpp
    notificationhandler.cpp
)

add_library(notificationplugin SHARED ${SRCS})

target_link_libraries(notificationplugin sink)
qt5_use_modules(notificationplugin Core Quick Qml)

install(TARGETS notificationplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/notifications)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/notifications)
