project(kube)

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
target_link_libraries(${PROJECT_NAME}
    Qt5::Gui
    Qt5::Quick
    Qt5::Widgets
    Qt5::WebEngine
    KF5::Package
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kube.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES org.kde.kube.appdata.xml DESTINATION ${KDE_INSTALL_DATAROOTDIR}/appdata/)
