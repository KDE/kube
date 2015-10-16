set(mailplugin_SRCS
    mailplugin.cpp
    maillistcontroller.cpp
    maillistmodel.cpp
)

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml)

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/mail)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/akonadi2/mail)