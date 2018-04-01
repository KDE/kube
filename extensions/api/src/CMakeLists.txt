add_definitions("-Wall -std=c++14 -g")
set(CMAKE_CXX_VISIBILITY_PRESET default)

find_package(Qt5 COMPONENTS REQUIRED Core Concurrent Quick Qml WebEngineWidgets Test WebEngine Gui)
find_package(KF5Mime 4.87.0 CONFIG REQUIRED)
find_package(Sink 0.6.0 CONFIG REQUIRED)

include_directories(../../../framework/src/domain/mime ${KMIME_INCLUDES})

add_library(extensionapiplugin SHARED extensionapiplugin.cpp extensionapi.cpp)
target_link_libraries(extensionapiplugin
    kubeframework
    KF5::Mime
    sink
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::WebEngineWidgets
    Qt5::Test
    Qt5::WebEngine
    Qt5::Gui
)
install(TARGETS extensionapiplugin DESTINATION ${EXTENSIONAPI_INSTALL_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
