project(kube-accounts-generic)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)

set(SRCS
    settings.cpp
    accountplugin.cpp
)

add_library(genericaccountplugin SHARED ${SRCS})
target_link_libraries(genericaccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

install(TARGETS genericaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/generic)
install_qml_account(generic)
