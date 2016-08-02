add_subdirectory(gnupg_home)
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )
include(${CMAKE_CURRENT_SOURCE_DIR}/kdepim_add_gpg_crypto_test.cmake)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

add_executable(mimetreeparsertest interfacetest.cpp)
add_gpg_crypto_test(mimetreeparsertest mimetreeparsertest)
qt5_use_modules(mimetreeparsertest Core Test)
target_link_libraries(mimetreeparsertest mimetreeparser)
