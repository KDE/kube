set(mailplugin_SRCS
    attachmentmodel.cpp
    mailplugin.cpp
    maillistmodel.cpp
    folderlistmodel.cpp
    actions/sinkactions.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
    composercontroller.cpp
    messageparser.cpp
    messageparser_new.cpp
    messageparser_old.cpp
    mailtemplates.cpp
    modeltest.cpp
    retriever.cpp
    outboxmodel.cpp
    identitiesmodel.cpp
    recepientautocompletionmodel.cpp
    settings/accountsettings.cpp
    selector.cpp
    completer.cpp
    controller.cpp
    outboxcontroller.cpp
    mailcontroller.cpp
    foldercontroller.cpp
    mouseproxy.cpp
    contactcontroller.cpp
    peoplemodel.cpp
)
find_package(KF5 REQUIRED COMPONENTS Package)

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml WebKitWidgets Test)
target_link_libraries(mailplugin actionplugin settingsplugin sink mimetreeparser KF5::MimeTreeParser KF5::Codecs KF5::Package KAsync)


install(TARGETS mailplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

add_subdirectory(actions/tests)
add_subdirectory(mimetreeparser)
