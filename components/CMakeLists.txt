project(kube-components)

cmake_minimum_required(VERSION 2.8.12)

include(CPack)
include(FeatureSummary)
find_package(PkgConfig)

################# set KDE specific information #################

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets)

set (QT_MIN_VERSION "5.4.0")
find_package(KF5 REQUIRED COMPONENTS Package)


macro(install_component name)
    install(DIRECTORY ${name}/contents/ui/ DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    install(FILES ${name}/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kube/components/${name})
    kpackage_install_package(${name} org.kube.components.${name} "genericqml")
endmacro(install_component)

install_component(mail)
install_component(accounts)
install_component(mailviewer)
