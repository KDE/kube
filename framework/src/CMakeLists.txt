
find_package(Qt5 COMPONENTS REQUIRED Core Concurrent Quick Qml WebEngineWidgets Test WebEngine Gui)
find_package(KF5Mime 4.87.0 CONFIG REQUIRED)
find_package(KF5CalendarCore CONFIG REQUIRED)
find_package(Sink 0.7.0 CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(Gpgme REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)
find_package(KF5Contacts CONFIG REQUIRED)

include(GenerateExportHeader)

set(CMAKE_CXX_VISIBILITY_PRESET default)

include_directories(. domain/mime/mimetreeparser domain/ domain/mime)

add_library(crypto STATIC
    crypto.cpp
)
target_link_libraries(crypto
    PUBLIC
    Qt5::Core
    Gpgme::Gpgme
)
add_library(kubeframework SHARED
    settings/settings.cpp
    domain/maillistmodel.cpp
    domain/folderlistmodel.cpp
    domain/perioddayeventmodel.cpp
    domain/eventoccurrencemodel.cpp
    domain/eventcontroller.cpp
    domain/invitationcontroller.cpp
    domain/todomodel.cpp
    domain/todocontroller.cpp
    domain/multidayeventmodel.cpp
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
    domain/textdocumenthandler.cpp
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
    clipboardproxy.cpp
    krecursivefilterproxymodel.cpp
    qquicktreemodeladaptor.cpp
    startupcheck.cpp
    keyring.cpp
    domainobjectcontroller.cpp
    extensionmodel.cpp
    viewhighlighter.cpp
    file.cpp
    logmodel.cpp
    entitymodel.cpp
    )
generate_export_header(kubeframework BASE_NAME Kube EXPORT_FILE_NAME kube_export.h)
set_target_properties(kubeframework PROPERTIES
    ENABLE_EXPORTS 1
    WINDOWS_EXPORT_ALL_SYMBOLS 1
)
target_link_libraries(kubeframework
    sink
    kube_otp
    mailcrypto
    crypto
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::WebEngineWidgets
    Qt5::Test
    Qt5::WebEngine
    Qt5::Gui
    KF5::Codecs
    KF5::Contacts
    KF5::CalendarCore
    KAsync
)
install(TARGETS kubeframework DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_library(frameworkplugin SHARED frameworkplugin.cpp)
target_link_libraries(frameworkplugin
    kubeframework
)
install(TARGETS frameworkplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

set(BUILD_TESTING ON)

add_subdirectory(tests)
add_subdirectory(domain/mime)
add_subdirectory(domain/mime/tests)
add_subdirectory(domain/mime/mimetreeparser)
add_subdirectory(domain/settings/tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
