set(mailplugin_SRCS
    mailplugin.cpp
    maillistcontroller.cpp
    maillistmodel.cpp
    singlemailcontroller.cpp
)

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml)

target_link_libraries(mailplugin /work/install/lib64/libakonadi2common.so)
#target_link_libraries(mailplugin /home/mike/projects/_install/lib/x86_64-linux-gnu/libakonadi2common.so)

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/mail)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/mail)