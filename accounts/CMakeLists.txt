include_directories(../framework/src/)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)

macro(install_qml_account name)
    install(DIRECTORY package/contents/ui/ DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/${name})
    install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/${name})
endmacro(install_qml_account)

add_subdirectory(maildir)
add_subdirectory(imap)
add_subdirectory(kolabnow)
add_subdirectory(gmail)
