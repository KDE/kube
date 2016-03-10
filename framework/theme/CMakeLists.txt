set(themeplugin_SRCS
    themeplugin.cpp
    colorpalette.cpp
)

add_library(themeplugin SHARED ${themeplugin_SRCS})

qt5_use_modules(themeplugin Core Quick Qml)

target_link_libraries(themeplugin)

install(TARGETS themeplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/theme)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/theme)
