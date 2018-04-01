macro(install_extension name extensionpoint)
    install(DIRECTORY ${name}/qml/ DESTINATION ${QML_INSTALL_DIR}/org/kube/extensions/${extensionpoint}/${name})
    #install(FILES ${name}/metadata.json DESTINATION ${QML_INSTALL_DIR}/org/kube/extensions/${name})
endmacro()

add_subdirectory(api)
install_extension(fileasexpense mailview)
