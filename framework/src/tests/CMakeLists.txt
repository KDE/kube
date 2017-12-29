include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Test Gui)

add_executable(folderlistmodeltest folderlistmodeltest.cpp)
add_test(folderlistmodeltest folderlistmodeltest)
target_link_libraries(folderlistmodeltest
    Qt5::Core
    Qt5::Test
    Qt5::Gui
    kubeframework
)

add_executable(maillistmodeltest maillistmodeltest.cpp)
add_test(maillistmodeltest maillistmodeltest)
target_link_libraries(maillistmodeltest
    Qt5::Core
    Qt5::Test
    Qt5::Gui
    kubeframework
)
