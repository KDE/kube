find_package(Qt5 REQUIRED NO_MODULE COMPONENTS QuickTest Network Quick)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5CalendarCore CONFIG REQUIRED)

add_executable(kubetestrunner kubetestrunner.cpp)
target_link_libraries(kubetestrunner
    Qt5::QuickTest
    Qt5::Quick
    sink
    kubeframework
)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/test)

add_library(testplugin SHARED testplugin.cpp teststore.cpp)
target_link_libraries(testplugin
    kubeframework
    KF5::CalendarCore
)
install(TARGETS testplugin DESTINATION ${QML_INSTALL_DIR}/org/kube/test)
