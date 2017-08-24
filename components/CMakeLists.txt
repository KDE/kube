project(kube-components)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets)
find_package(KF5 REQUIRED COMPONENTS Package)

macro(install_component name)
    install(DIRECTORY ${name}/contents/ui/ DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    install(FILES ${name}/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    kpackage_install_package(${name} org.kube.components.${name} "genericqml")
endmacro(install_component)

install_component(kube)
install_component(accounts)
install_component(mailviewer)
