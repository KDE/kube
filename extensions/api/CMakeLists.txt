include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)

set(EXTENSIONAPI_INSTALL_DIR ${QML_INSTALL_DIR}/org/kube/extensionapi)

install(DIRECTORY qml/ DESTINATION ${EXTENSIONAPI_INSTALL_DIR})
install(FILES qmldir DESTINATION ${EXTENSIONAPI_INSTALL_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
