include_directories(../)
add_executable(imapsettingstest settingstest.cpp)
add_test(imapsettingstest settingstest)
target_link_libraries(imapsettingstest
    sink
    imapaccount_static
    Qt5::Core
    Qt5::Concurrent
    Qt5::Test
)
