find_package(Qt5 REQUIRED NO_MODULE COMPONENTS QuickTest Network)
find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)

add_executable(kubetestrunner kubetestrunner.cpp)
target_link_libraries(kubetestrunner
    Qt5::QuickTest
    sink
)
