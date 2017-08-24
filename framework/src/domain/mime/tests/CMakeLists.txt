add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../testdata" )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Test WebEngine)

include( ${CMAKE_SOURCE_DIR}/cmake/modules/add_gpg_crypto_test.cmake )

add_executable(mailtemplatetest mailtemplatetest.cpp)
add_gpg_crypto_test(mailtemplatetest mailtemplatetest)
target_link_libraries(mailtemplatetest
    Qt5::Core
    Qt5::Test
    Qt5::WebEngine
    frameworkplugin
)
