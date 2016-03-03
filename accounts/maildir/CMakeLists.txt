project(kube-accounts-maildir)

cmake_minimum_required(VERSION 2.8.10.1)

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

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)

set (QT_MIN_VERSION "5.4.0")
find_package(KF5 REQUIRED COMPONENTS Package)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/kube/accounts/maildir)
install(FILES package/contents/ui/MaildirAccountSettings.qml DESTINATION ${QML_INSTALL_DIR}/org/kde/kube/accounts/maildir)

