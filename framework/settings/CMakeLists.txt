set(settingsplugin_SRCS
    settingsplugin.cpp
    settings.cpp
)

add_library(settingsplugin SHARED ${settingsplugin_SRCS})

qt5_use_modules(settingsplugin Core Quick Qml)

target_link_libraries(settingsplugin)

install(TARGETS settingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/settings)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/settings)
