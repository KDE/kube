set(mailplugin_SRCS
    mailplugin.cpp
    maillistmodel.cpp
    folderlistmodel.cpp
    actions/sinkactions.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
    composercontroller.cpp
    messageparser.cpp
    mailtemplates.cpp
    retriever.cpp
    accountfactory.cpp
    accountscontroller.cpp
    accountsmodel.cpp
    identitiesmodel.cpp
    settings/accountsettings.cpp
)
find_package(KF5 REQUIRED COMPONENTS Package)

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml WebKitWidgets)
target_link_libraries(mailplugin actionplugin settingsplugin sink mimetreeparser KF5::MimeTreeParser KF5::Codecs KF5::Package KF5::Async KF5::IconThemes)

add_subdirectory(actions/tests)

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)

add_subdirectory(mimetreeparser)