
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Quick QuickTest)

add_executable(testrunner testrunner.cpp)
target_link_libraries(testrunner
    Qt5::Quick
    Qt5::QuickTest
)

add_test(NAME frameworkqmltests COMMAND testrunner WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

