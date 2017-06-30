add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../testdata" )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

include(ECMAddTests)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Test WebEngine)

ecm_add_test(mailtemplatetest.cpp
    TEST_NAME "mailtemplatetest"
    NAME_PREFIX ""
    LINK_LIBRARIES Qt5::Core Qt5::Test Qt5::WebEngine frameworkplugin
)
