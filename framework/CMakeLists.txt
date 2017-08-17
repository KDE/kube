cmake_minimum_required(VERSION 2.8.12)


# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
        ${CMAKE_MODULE_PATH})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(KDEInstallDirs)

set(CMAKE_AUTOMOC ON)

enable_testing()

set(FRAMEWORK_INSTALL_DIR ${QML_INSTALL_DIR}/org/kube/framework)

install(DIRECTORY qml/ DESTINATION ${FRAMEWORK_INSTALL_DIR})
install(FILES qmldir DESTINATION ${FRAMEWORK_INSTALL_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
