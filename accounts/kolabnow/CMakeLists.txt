project(kube-accounts-kolabnow)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

set(SRCS
    kolabnowsettings.cpp
    kolabnowaccountplugin.cpp
)

add_library(kolabnowaccountplugin SHARED ${SRCS})
target_link_libraries(kolabnowaccountplugin
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)

add_library(kolabnowaccount_static STATIC ${SRCS})
target_link_libraries(kolabnowaccount_static
    sink
    frameworkplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
)
# add_subdirectory(tests)

kpackage_install_package(package org.kube.accounts.kolabnow "genericqml")

install(TARGETS kolabnowaccountplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/kolabnow)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/accounts/kolabnow)
