include_directories(../)
add_executable(settingstest settingstest.cpp)
add_test(settingstest settingstest)
target_link_libraries(settingstest
    frameworkplugin
    Qt5::Core
    Qt5::Concurrent
    Qt5::Test
)
