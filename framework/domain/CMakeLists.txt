set(mailplugin_SRCS
    mailplugin.cpp
    maillistmodel.cpp
    folderlistmodel.cpp
    actions/sinkactions.cpp
    actions/mailactions.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
    csshelper.cpp
    composercontroller.cpp
    messageparser.cpp
    mailtransport.cpp
    mailtemplates.cpp
    retriever.cpp
    accountfactory.cpp
    accountscontroller.cpp
    accountsmodel.cpp
)
add_definitions(-DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

find_package(CURL 7.20.0 REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package)

include_directories(${CURL_INCLUDE_DIRS})

add_library(mailplugin SHARED ${mailplugin_SRCS})

qt5_use_modules(mailplugin Core Quick Qml WebKitWidgets)
target_link_libraries(mailplugin actionplugin settingsplugin sink KF5::MimeTreeParser KF5::Codecs KF5::Package ${CURL_LIBRARIES})

install(TARGETS mailplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/framework/domain)
