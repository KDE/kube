include_directories(../)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${GPGME_INCLUDE_DIRS})

set(AUTOMOC ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../testdata" )

include(ECMAddTests)
include(${CMAKE_SOURCE_DIR}/cmake/modules/add_gpg_crypto_test.cmake)

# convenience macro to add qtest unit tests
macro(add_mimetreeparser_unittest _source)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} util.cpp setupenv.cpp
        TEST_NAME ${_name}
        NAME_PREFIX "mimetreeparser-"
        LINK_LIBRARIES kube_otp Qt5::Test KF5::Mime
    )
endmacro ()

macro(add_mimetreeparser_class_unittest _source _additionalSource)
    get_filename_component(_name ${_source} NAME_WE)
    ecm_add_test(${_source} ${_additionalSource}
        TEST_NAME ${_name}
        NAME_PREFIX "mimetreeparser-"
        LINK_LIBRARIES kube_otp Qt5::Test KF5::Mime
    )
endmacro ()

macro(add_mimetreeparser_crypto_unittest _source)
    set(_test ${_source} util.cpp)
    get_filename_component(_name ${_source} NAME_WE)
    add_executable( ${_name} ${_test} setupenv.cpp)
    ecm_mark_as_test(${_name})
    target_link_libraries( ${_name}
        kube_otp
        Qt5::Test
        KF5::Mime
        Gpgme::Gpgme
    )
    add_gpg_crypto_test(${_name} mimetreeparser-${_name})
endmacro ()

add_mimetreeparser_crypto_unittest(attachmenttest.cpp)
add_mimetreeparser_unittest(nodehelpertest.cpp)
add_mimetreeparser_class_unittest(cryptohelpertest.cpp "../cryptohelper.cpp")
