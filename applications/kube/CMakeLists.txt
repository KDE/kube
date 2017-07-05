project(kube)

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0063 NEW)

include(CPack)
include(FeatureSummary)

find_package(PkgConfig)

################# set KDE specific information #################

if(APPLE OR WIN32)
    # for an ecm_add_app_icon() implementation acception svg icons
    set(ECM_MIN_VERSION 5.29.0)
else()
    set(ECM_MIN_VERSION 0.0.8)
endif()
find_package(ECM ${ECM_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMAddAppIcon)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets WebEngine)

find_package(KF5 REQUIRED COMPONENTS Package)

# install executable
set(SRCS main.cpp)

if(APPLE OR WIN32)
    # Sets the icon on Windows and OSX
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../icons/*-apps-kube_icon.*")
    ecm_add_app_icon(SRCS ICONS ${ICONS_SRCS})
endif()

add_executable(${PROJECT_NAME} ${SRCS})
qt5_use_modules(${PROJECT_NAME} Gui Quick Widgets WebEngine)
target_link_libraries(${PROJECT_NAME} KF5::Package)

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kube.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES org.kde.kube.appdata.xml DESTINATION ${KDE_INSTALL_DATAROOTDIR}/appdata/)
