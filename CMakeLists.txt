cmake_minimum_required(VERSION 3.0)

project(kube VERSION 0.6)

set (QT_MIN_VERSION "5.9.0")

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0063 NEW)

option(EXPERIMENTAL_VIEWS "Install experimental views" OFF)

include(CPack)
include(FeatureSummary)
find_package(PkgConfig REQUIRED)
find_package(ECM 5.29.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
#Avoid building appstreamtest
set(KDE_SKIP_TEST_SETTINGS true)
include(KDECMakeSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)

ecm_setup_version(PROJECT
                  SOVERSION kube_VERSION_MAJOR
                  VERSION_HEADER kube_version.h
                  )

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(framework)
add_subdirectory(components)
add_subdirectory(icons)
add_subdirectory(applications)
add_subdirectory(views)
add_subdirectory(accounts)
add_subdirectory(tests)
