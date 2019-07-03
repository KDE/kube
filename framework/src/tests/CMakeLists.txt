include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Test Gui)

macro(auto_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(${_testname} ${_testname})
        target_link_libraries(${_testname}
            Qt5::Core
            Qt5::Test
            Qt5::Gui
            kubeframework
        )
    endforeach(_testname)
endmacro(auto_tests)

auto_tests(
    folderlistmodeltest
    maillistmodeltest
    eventoccurrencemodeltest
    entitymodeltest
    invitationcontrollertest
)
