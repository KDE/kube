include(ECMAddAppIcon)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Quick)

# install executable
set(SRCS main.cpp backtrace.cpp)

if(APPLE OR WIN32)
    # Sets the icon on Windows and OSX
    file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../icons/*-apps-kube_icon.png")
    ecm_add_app_icon(SRCS ICONS ${ICONS_SRCS})
endif()

add_executable(${PROJECT_NAME} ${SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST MacOSXBundleInfo.plist.in
    MACOSX_BUNDLE_INFO_STRING "A modern communication and collaboration client."
    #MACOSX_BUNDLE_ICON_FILE already set by ecm_add_app_icon
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.kubeproject.kube"
    MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_NAME "Kube"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_COPYRIGHT "Free and open-source under the GPL"
    )

target_link_libraries(${PROJECT_NAME}
    Qt5::Quick
    ${CMAKE_DL_LIBS}
    kubeframework
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

set(KUBE_APPLICATION_ID "org.kde.kube" CACHE STRING "The application identifier used for desktopfile and appdata")

set(KUBE_DESKTOPFILE_NAME "Kube" CACHE STRING "desktopfile name property")
set(KUBE_DESKTOPFILE_GENERICNAME "Mail Client" CACHE STRING "desktopfile genericname property")
set(KUBE_DESKTOPFILE_EXEC "kube" CACHE STRING "desktopfile exec property")
set(KUBE_DESKTOPFILE_COMMENT "Send, receive and manage your mail with Kube" CACHE STRING "desktopfile comment property")
set(KUBE_DESKTOPFILE_CATEGORIES "Qt;KDE;Office;Network;Email;" CACHE STRING "desktopfile categories property")
set(KUBE_DESKTOPFILENAME "${KUBE_APPLICATION_ID}.desktop")
configure_file(kube.desktop.in ${KUBE_DESKTOPFILENAME})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${KUBE_DESKTOPFILENAME} DESTINATION ${KDE_INSTALL_APPDIR} )

set(KUBE_APPDATA_ID "${KUBE_DESKTOPFILENAME}" CACHE STRING "appdata homepage property")
set(KUBE_APPDATA_HOMEPAGE "https://kube-project.com" CACHE STRING "appdata homepage property")
set(KUBE_APPDATA_BUGTRACKER "https://phabricator.kde.org/project/view/238/" CACHE STRING "appdata bugtracker property")
set(KUBE_APPDATA_HELP "https://kube.readthedocs.io/en/latest/" CACHE STRING "appdata help property")
set(KUBE_APPDATA_PROJECTGROUP "KDE" CACHE STRING "appdata projectgroup property")
set(KUBE_APPDATAFILENAME "${KUBE_APPLICATION_ID}.appdata.xml")
configure_file(kube.appdata.xml.in ${KUBE_APPDATAFILENAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${KUBE_APPDATAFILENAME} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/appdata/)
