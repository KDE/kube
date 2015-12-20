set(settingsplugin_SRCS
    settingsplugin.cpp
    maildir_resource.cpp
    resourcelistmodel.cpp
    resourcescontroller.cpp
)

add_library(settingsplugin SHARED ${settingsplugin_SRCS})

qt5_use_modules(settingsplugin Core Quick Qml)

target_link_libraries(settingsplugin /work/install/lib64/libakonadi2common.so)
#target_link_libraries(settingsplugin /home/mike/projects/_install/lib/x86_64-linux-gnu/libakonadi2common.so)

install(TARGETS settingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/settings)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/settings)