find_package(Qt5 REQUIRED NO_MODULE COMPONENTS QuickTest)

add_executable(kubetestrunner kubetestrunner.cpp)
target_link_libraries(kubetestrunner
    Qt5::QuickTest
)
