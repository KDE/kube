add_subdirectory(gnupg_home)
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../testdata" )
include(${CMAKE_SOURCE_DIR}/cmake/modules/add_gpg_crypto_test.cmake)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

include(ECMAddTests)

find_package(Gpgmepp 1.7.1 CONFIG)
find_package(QGpgme 1.7.1 CONFIG)

add_executable(mimetreeparsertest mimetreeparsertest.cpp)
add_gpg_crypto_test(mimetreeparsertest mimetreeparsertest)
target_link_libraries(mimetreeparsertest
    kube_otp
    Qt5::Core
    Qt5::Test
    KF5::Mime
    Gpgmepp
    QGpgme
)

ecm_add_test(gpgerrortest.cpp
    TEST_NAME "gpgerrortest"
    NAME_PREFIX "mimetreeparser-"
    LINK_LIBRARIES Qt5::Core Qt5::Test kube_otp Gpgmepp QGpgme
)
