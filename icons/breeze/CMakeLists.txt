include(KDEInstallDirs)

set(BINARY_ICONS_RESOURCE TRUE)
if(BINARY_ICONS_RESOURCE)
    find_package(Qt5 NO_MODULE REQUIRED Core)
    add_executable(qrcAlias qrcAlias.cpp)
    target_link_libraries(qrcAlias PUBLIC Qt5::Core)

    function(generate_binary_resource target outfile)
        set(RESOURCES_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR}/res)
        set(RESOURCE_FILE ${RESOURCES_WORKING_DIR}/kube-${target}.qrc)
        set(BINARY_RESOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/kube-${target}.rcc)

        get_target_property(QT_RCC_EXECUTABLE Qt5::rcc LOCATION)

        file(REMOVE ${BINARY_RESOURCE_FILE})
        file(REMOVE_RECURSE ${RESOURCES_WORKING_DIR})
        file(MAKE_DIRECTORY ${RESOURCES_WORKING_DIR})
        #Copying symlinks doesn't work on windows if the target is not yet available.
        #We use execute_process because we expect failures and don't want to abort the script.
        #We copy two times so we can deal with symlinks 2 levels deep.
        #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${target}/ DESTINATION ${RESOURCES_WORKING_DIR})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${target}/ ${RESOURCES_WORKING_DIR})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${target}/ ${RESOURCES_WORKING_DIR})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${target}/ ${RESOURCES_WORKING_DIR})
        file(REMOVE
            ${RESOURCE_FILE}
            ${RESOURCE_FILE}.depends
            ${RESOURCES_WORKING_DIR}/.gitignore
            ${RESOURCES_WORKING_DIR}/CMakeLists.txt
        )

        add_custom_command(OUTPUT ${RESOURCE_FILE} always_rebuild1
            COMMAND ${QT_RCC_EXECUTABLE} --project -o ${CMAKE_CURRENT_BINARY_DIR}/tmp.qrc
            COMMAND $<TARGET_FILE:qrcAlias> -i ${CMAKE_CURRENT_BINARY_DIR}/tmp.qrc -o ${RESOURCE_FILE}
            DEPENDS ${RESOURCES_WORKING_DIR}
            WORKING_DIRECTORY ${RESOURCES_WORKING_DIR}
            COMMENT "Generating ${RESOURCE_FILE}"
        )
        add_custom_command(OUTPUT ${BINARY_RESOURCE_FILE} always_rebuild2
            COMMAND ${QT_RCC_EXECUTABLE} --binary
                        -o ${BINARY_RESOURCE_FILE}
                        ${RESOURCE_FILE}
            DEPENDS ${RESOURCE_FILE} always_rebuild1
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating ${BINARY_RESOURCE_FILE}"
        )

        add_custom_target(kube-${target}-rcc ALL DEPENDS ${BINARY_RESOURCE_FILE} always_rebuild2)

        set(${outfile} ${BINARY_RESOURCE_FILE} PARENT_SCOPE)
    endfunction()

    generate_binary_resource(icons binary_resource)
    install(FILES ${binary_resource} DESTINATION ${KDE_INSTALL_DATADIR}/kube)
else()
    install(DIRECTORY
        icons/places
        icons/actions
        icons/mimetypes
        DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
    install(FILES icons/index.theme DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
endif()
