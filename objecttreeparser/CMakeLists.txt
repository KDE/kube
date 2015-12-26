project(objecttreeparser)

cmake_minimum_required(VERSION 2.8.12)

# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 COMPONENTS REQUIRED Core Gui)
# find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/common)
include_directories(SYSTEM /work/install/include/)
include_directories(SYSTEM /work/install/include/KF5/)

enable_testing()

add_library(${PROJECT_NAME} objecttreeparser.cpp)
qt5_use_modules(${PROJECT_NAME} Gui)
target_link_libraries(${PROJECT_NAME} KF5::Package KF5::Mime)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
