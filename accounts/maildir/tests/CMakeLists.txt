include_directories(../)
add_executable(maildirsettingstest settingstest.cpp)
add_test(maildirsettingstest maildirsettingstest)
target_link_libraries(maildirsettingstest
    sink
    maildiraccount_static
    Qt5::Core
    Qt5::Test
    Qt5::Concurrent
)
