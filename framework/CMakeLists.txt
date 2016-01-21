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
find_package(Akonadi2Common CONFIG REQUIRED)
find_package(KF5Async)
find_package(KF5Libkleo)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(.)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/common)
include_directories(SYSTEM /work/install/include/)
include_directories(SYSTEM /work/install/include/KF5/)
include_directories(SYSTEM /work/install/include/KF5/KIconThemes)
include_directories(SYSTEM /work/install/include/KF5/KMime)
include_directories(SYSTEM /work/install/include/KF5/KService)
include_directories(SYSTEM /work/install/include/KF5/KCoreAddons)

enable_testing()

set(AKONADI2_RESOURCE_PLUGINS_PATH ${QT_PLUGIN_INSTALL_DIR}/akonadi2/resources)

add_subdirectory(mail)
add_subdirectory(actions)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
