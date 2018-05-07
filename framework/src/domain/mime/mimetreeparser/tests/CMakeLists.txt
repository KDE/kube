add_subdirectory(gnupg_home)
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../testdata" )
include(${CMAKE_SOURCE_DIR}/cmake/modules/add_gpg_crypto_test.cmake)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

include(ECMAddTests)

add_executable(mimetreeparsertest mimetreeparsertest.cpp)
add_gpg_crypto_test(mimetreeparsertest mimetreeparsertest)
target_link_libraries(mimetreeparsertest
    kube_otp
    Qt5::Core
    Qt5::Test
    KF5::Mime
    ${GPGME_LIBRARIES}
)

ecm_add_test(gpgerrortest.cpp
    TEST_NAME "gpgerrortest"
    NAME_PREFIX "mimetreeparser-"
    LINK_LIBRARIES Qt5::Core Qt5::Test kube_otp ${GPGME_LIBRARIES}
)
