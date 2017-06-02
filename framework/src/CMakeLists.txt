
find_package(Qt5 COMPONENTS REQUIRED Core Quick Qml WebEngineWidgets Test)
find_package(KF5Mime "4.87.0" CONFIG REQUIRED)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(QGpgme CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)
find_package(KF5Package CONFIG REQUIRED)
find_package(KF5Contacts CONFIG REQUIRED)

add_definitions("-Wall -std=c++0x -g")

include_directories(. domain/mime/mimetreeparser domain/ domain/mime)

set(SRCS
    frameworkplugin.cpp
    settings/settings.cpp
    domain/maillistmodel.cpp
    domain/folderlistmodel.cpp
    domain/composercontroller.cpp
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
    domain/mime/htmlutils.cpp
    domain/mime/messageparser.cpp
    domain/mime/attachmentmodel.cpp
    domain/mime/partmodel.cpp
    domain/mime/mailtemplates.cpp
    accounts/accountfactory.cpp
    accounts/accountsmodel.cpp
    fabric.cpp
    sinkfabric.cpp
    kubeimage.cpp
)

add_library(frameworkplugin SHARED ${SRCS})
target_link_libraries(frameworkplugin
    sink
    kube_otp
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::WebEngineWidgets
    Qt5::Test
    KF5::Codecs
    KF5::Contacts
    KF5::Package
    KAsync
)
install(TARGETS frameworkplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

set(BUILD_TESTING ON)

add_subdirectory(domain/mime/tests)
add_subdirectory(domain/mime/mimetreeparser)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
