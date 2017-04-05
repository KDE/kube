set(settingsplugin_SRCS
    settingsplugin.cpp
    settings.cpp
)

add_library(settingsplugin SHARED ${settingsplugin_SRCS})

qt5_use_modules(settingsplugin Core Quick Qml)

target_link_libraries(settingsplugin)

install(TARGETS settingsplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})
