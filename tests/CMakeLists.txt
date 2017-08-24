project(tests)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets WebEngine QuickTest)

# build testrunner
add_executable(kubetestrunner kubetestrunner.cpp)
target_link_libraries(kubetestrunner
    Qt5::Gui
    Qt5::Quick
    Qt5::Widgets
    Qt5::WebEngine
    Qt5::QuickTest
)

# tests
add_test(NAME qmltests COMMAND kubetestrunner WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

