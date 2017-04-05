cmake_minimum_required(VERSION 2.8.12)


# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
        ${CMAKE_MODULE_PATH})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(KDEInstallDirs)

find_package(Qt5 COMPONENTS REQUIRED Core Qml)

find_package(KF5MimeTreeParser "5.1.46" CONFIG REQUIRED)
find_package(KF5Mime "4.87.0" CONFIG REQUIRED)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(QGpgme CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)
find_package(KF5Package CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(.)
include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)
include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/KMime)

enable_testing()

set(SINK_RESOURCE_PLUGINS_PATH ${QT_PLUGIN_INSTALL_DIR}/sink/resources)
set(FRAMEWORK_INSTALL_DIR ${QML_INSTALL_DIR}/org/kube/framework)

install(DIRECTORY qml/ DESTINATION ${FRAMEWORK_INSTALL_DIR})
install(FILES qmldir DESTINATION ${FRAMEWORK_INSTALL_DIR})

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
    domain/controller.cpp
    domain/outboxcontroller.cpp
    domain/mailcontroller.cpp
    domain/foldercontroller.cpp
    domain/mouseproxy.cpp
    domain/contactcontroller.cpp
    domain/peoplemodel.cpp
    accounts/accountfactory.cpp
    accounts/accountsmodel.cpp
    notifications/notificationhandler.cpp
)

add_library(frameworkplugin SHARED ${SRCS})

qt5_use_modules(frameworkplugin Core Quick Qml WebKitWidgets Test)
target_link_libraries(frameworkplugin sink mimetreeparser KF5::MimeTreeParser KF5::Codecs KF5::Package KAsync)

install(TARGETS frameworkplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

# The actions framework
#add_subdirectory(actions)
# The settings framework
#add_subdirectory(settings)
# Domain specific domain logic
#add_subdirectory(domain)
# The accounts framework
#add_subdirectory(accounts)
# The notifications framework
#add_subdirectory(notifications)
add_subdirectory(domain/actions/tests)
add_subdirectory(domain/mimetreeparser)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
