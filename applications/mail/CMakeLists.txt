project(kube-mail)

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0063 NEW)

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

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets WebEngine)

set (QT_MIN_VERSION "5.6.0")
find_package(KF5 REQUIRED COMPONENTS Package)

# install executable
add_executable(${PROJECT_NAME} main.cpp)
qt5_use_modules(${PROJECT_NAME} Gui Quick Widgets WebEngine)
target_link_libraries(${PROJECT_NAME} KF5::Package)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${kube_VERSION})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

install(PROGRAMS org.kde.kube.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES org.kde.kube.appdata.xml DESTINATION ${KDE_INSTALL_DATAROOTDIR}/appdata/)
