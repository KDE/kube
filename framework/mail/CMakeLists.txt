set(mailplugin_SRCS
    mailplugin.cpp
    maillistcontroller.cpp
    maillistmodel.cpp
    singlemailcontroller.cpp
    folderlistmodel.cpp
    folderlistcontroller.cpp
    actions/sinkactions.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
)

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml)

target_link_libraries(mailplugin actionplugin sinkcommon KF5::Otp)

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/kube/mail)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/kube/mail)
