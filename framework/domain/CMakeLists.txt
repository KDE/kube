set(mailplugin_SRCS
    mailplugin.cpp
    maillistcontroller.cpp
    maillistmodel.cpp
    singlemailcontroller.cpp
    folderlistmodel.cpp
    folderlistcontroller.cpp
    actions/sinkactions.cpp
    actions/mailactions.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
    csshelper.cpp
    composer.cpp
    messageparser.cpp
    mailtransport.cpp
    mailtemplates.cpp
    retriever.cpp
)
add_definitions(-DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

find_package(CURL 7.20.0 REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml WebKitWidgets)

target_link_libraries(mailplugin actionplugin settingsplugin sink KF5::Otp KF5::Codecs ${CURL_LIBRARIES})

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)
