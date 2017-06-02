include_directories(../)
cmake_policy(SET CMP0063 NEW)
add_executable(settingstest settingstest.cpp)
add_test(settingstest settingstest)
target_link_libraries(settingstest
    sink
    maildiraccount_static
    Qt5::Core
    Qt5::Test
    Qt5::Concurrent
)
