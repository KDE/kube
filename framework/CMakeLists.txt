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

find_package(Qt5 COMPONENTS REQUIRED Core Qml)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/common)

enable_testing()

set(AKONADI2_RESOURCE_PLUGINS_PATH ${QT_PLUGIN_INSTALL_DIR}/akonadi2/resources)

add_subdirectory(mail)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
