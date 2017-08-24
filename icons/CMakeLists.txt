include(ECMInstallIcons)
include(KDEInstallDirs)

#Install kube icons
ecm_install_icons(ICONS sc-apps-kube_icon.svg 256-apps-kube_icon.png DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_symbol.svg DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_logo.svg DESTINATION share/icons)

#See the copybreeze script to adjust icons and to copy them over
install(DIRECTORY
    breeze/icons/places
    breeze/icons/actions
    breeze/icons/mimetypes
    DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
install(FILES breeze/icons/index.theme DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
