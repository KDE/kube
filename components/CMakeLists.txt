project(kube-components)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets)

macro(install_qml_component name)
    install(DIRECTORY ${name}/qml/ DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    install(FILES ${name}/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
endmacro(install_qml_component)

install_qml_component(kube)
add_subdirectory(kube/tests)
install_qml_component(accounts)


#Convert list to comma-separated and quoted list.
set(_AVAILABLE_ACCOUNT_PLUGINS ${AVAILABLE_ACCOUNT_PLUGINS})
set(AVAILABLE_ACCOUNT_PLUGINS "")
foreach(PLUGIN ${_AVAILABLE_ACCOUNT_PLUGINS})
    set(AVAILABLE_ACCOUNT_PLUGINS "${AVAILABLE_ACCOUNT_PLUGINS}, \"${PLUGIN}\"")
endforeach()
string(SUBSTRING "${AVAILABLE_ACCOUNT_PLUGINS}" 2 -1 AVAILABLE_ACCOUNT_PLUGINS)

configure_file(accounts/AccountConfiguration.qml.in accounts/AccountConfiguration.qml)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/accounts/AccountConfiguration.qml DESTINATION ${QML_INSTALL_DIR}/org/kube/components/accounts)
install_qml_component(mailviewer)
