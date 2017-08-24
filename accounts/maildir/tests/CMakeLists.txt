include_directories(../)
add_executable(settingstest settingstest.cpp)
add_test(settingstest settingstest)
target_link_libraries(settingstest
    sink
    maildiraccount_static
    Qt5::Core
    Qt5::Test
    Qt5::Concurrent
)
