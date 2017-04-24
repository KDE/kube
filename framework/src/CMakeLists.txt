
find_package(Qt5 COMPONENTS REQUIRED Core Qml)
find_package(KF5MimeTreeParser "5.1.46" CONFIG REQUIRED)
find_package(KF5Mime "4.87.0" CONFIG REQUIRED)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(QGpgme CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)
find_package(KF5Package CONFIG REQUIRED)

add_definitions("-Wall -std=c++0x -g")

include_directories(.)

set(SRCS
    frameworkplugin.cpp
    actions/action.cpp
    actions/actionhandler.cpp
    actions/actionbroker.cpp
    actions/actionresult.cpp
    actions/context.cpp
    settings/settings.cpp
    domain/attachmentmodel.cpp
    domain/maillistmodel.cpp
    domain/folderlistmodel.cpp
    domain/actions/sinkactions.cpp
    domain/objecttreesource.cpp
    domain/stringhtmlwriter.cpp
    domain/composercontroller.cpp
    domain/messageparser.cpp
    domain/messageparser_new.cpp
    domain/messageparser_old.cpp
    domain/mailtemplates.cpp
    domain/modeltest.cpp
    domain/retriever.cpp
    domain/outboxmodel.cpp
    domain/identitiesmodel.cpp
    domain/recepientautocompletionmodel.cpp
    domain/settings/accountsettings.cpp
    domain/selector.cpp
    domain/completer.cpp
    domain/mouseproxy.cpp
    domain/contactcontroller.cpp
    domain/controller.cpp
    domain/peoplemodel.cpp
    accounts/accountfactory.cpp
    accounts/accountsmodel.cpp
    fabric.cpp
    sinkfabric.cpp
)

add_library(frameworkplugin SHARED ${SRCS})
qt5_use_modules(frameworkplugin Core Quick Qml WebEngineWidgets Test)
target_link_libraries(frameworkplugin sink mimetreeparser KF5::MimeTreeParser KF5::Codecs KF5::Package KAsync)
install(TARGETS frameworkplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

add_subdirectory(domain/actions/tests)
add_subdirectory(domain/mimetreeparser)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
