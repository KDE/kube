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

find_package(KF5Otp "5.1.42" CONFIG REQUIRED)
find_package(KF5Mime "4.87.0" CONFIG REQUIRED)
find_package(Sink CONFIG REQUIRED)
find_package(KF5Async CONFIG REQUIRED)
find_package(KF5Libkleo CONFIG REQUIRED)
find_package(KF5IconThemes CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(.)
include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/KMime)
include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/KIconThemes)

enable_testing()

set(SINK_RESOURCE_PLUGINS_PATH ${QT_PLUGIN_INSTALL_DIR}/sink/resources)

# The actions framework
add_subdirectory(actions)
# The settings framework
add_subdirectory(settings)
# Domain specific domain logic
add_subdirectory(domain)
# Theme specfic things like colors
add_subdirectory(theme)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
