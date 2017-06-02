include_directories(${CMAKE_CURRENT_BINARY_DIR})
cmake_policy(SET CMP0063 NEW)
add_executable(sinkactiontest sinkactiontest.cpp)
add_test(sinkactiontest sinkactiontest)
target_link_libraries(sinkactiontest
    sink
    actionplugin
    mailplugin
    Qt5::Core
    Qt5::Concurrent
    Qt5::Test
    KF5::Mime
)
