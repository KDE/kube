project(kube-components)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets)

macro(install_qml_component name)
    install(DIRECTORY ${name}/qml/ DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    install(FILES ${name}/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
endmacro(install_qml_component)

install_qml_component(kube)
install_qml_component(accounts)
install_qml_component(mailviewer)
