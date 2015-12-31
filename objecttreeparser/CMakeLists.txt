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
include(ECMQtDeclareLoggingCategory)

find_package(Qt5 COMPONENTS REQUIRED Core Gui WebKitWidgets Widgets)
# find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui)
find_package(KF5 REQUIRED COMPONENTS Package Mime Codecs Config I18n Libkleo)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x -g")
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/common)
include_directories(SYSTEM /work/install/include/)
include_directories(SYSTEM /work/install/include/KF5/)
include_directories(${KF5Codecs_INCLUDE_DIRS} ${KF5ConfigCore_INCLUDE_DIRS})
include_directories(/work/install/include/KF5/KConfigCore/)
include_directories(/work/install/include/KF5/KI18n/)
include_directories(.)

enable_testing()

# ecm_qt_declare_logging_category(objecttreeparser.cpp HEADER messageviewer_debug.h IDENTIFIER MESSAGEVIEWER_LOG CATEGORY_NAME log_messageviewer)

add_library(${PROJECT_NAME}
    objecttreeparser.cpp
    attachmentstrategy.cpp
    bodypartformatterfactory.cpp
    converthtmltoplaintext.cpp
    cryptohelper.cpp
    csshelper.cpp
    htmlquotecolorer.cpp
    nodehelper.cpp
    messagecore/nodehelper.cpp
    messageviewerutil.cpp
    partnodebodypart.cpp
    messagepart.cpp
    messagecore/stringutil.cpp
    iconnamecache.cpp
)
qt5_use_modules(${PROJECT_NAME} Gui WebKitWidgets)
target_link_libraries(${PROJECT_NAME} KF5::Package KF5::Mime KF5::Codecs KF5::ConfigCore KF5::I18n KF5::Libkleo)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
