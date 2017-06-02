project(tests)

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0063 NEW)

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

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Test Gui Widgets WebEngine QuickTest)

# build testrunner
add_executable(kubetestrunner kubetestrunner.cpp)
target_link_libraries(kubetestrunner
    Qt5::Gui
    Qt5::Quick
    Qt5::Widgets
    Qt5::WebEngine
    Qt5::QuickTest
)

# tests
add_test(NAME qmltests COMMAND kubetestrunner WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

