cmake_minimum_required(VERSION 3.0)

project(kube VERSION 0.8)

set(QT_MIN_VERSION 5.9.0)

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0063 NEW)

option(EXPERIMENTAL_VIEWS "Install experimental views" OFF)
#Do not enable this unless you actually distribute a custom extension.
option(ENABLE_EXTENSION "Enable custom kube extensions" OFF)
set(AVAILABLE_ACCOUNT_PLUGINS "kolabnow" "imap" "maildir" "gmail" "generic" CACHE STRING "List of enabled account plugins (provide as semicolon separated string)" )

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

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

ecm_setup_version(PROJECT SOVERSION kube_VERSION_MAJOR)
configure_file(kube_version.h.in ${CMAKE_BINARY_DIR}/kube_version.h)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(framework)
add_subdirectory(components)
add_subdirectory(icons)
add_subdirectory(applications)
add_subdirectory(views)
add_subdirectory(accounts)
add_subdirectory(tests)
if (${ENABLE_EXTENSION})
    add_subdirectory(extensions)
endif()
