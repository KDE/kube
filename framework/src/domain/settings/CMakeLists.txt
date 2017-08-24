include_directories(${CMAKE_CURRENT_BINARY_DIR})
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
