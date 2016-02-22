find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMInstallIcons)

ecm_install_icons(ICONS sc-apps-kube_icon.svg DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_symbol.svg DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_logo.svg DESTINATION share/icons)
