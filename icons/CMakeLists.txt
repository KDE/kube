find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMInstallIcons)
include(KDEInstallDirs)

function(copy_breeze_icons)
    set(_multiValueArgs ICONS)
    cmake_parse_arguments(ARG "" "" "${_multiValueArgs}" ${ARGN} )
    set(_BREEZEICONS_DIR ${CMAKE_CURRENT_BINARY_DIR}/breeze)

    function(_copy_breeze_icon icon)
        SET(paths "")
        #breeze only ships svgs anyways
        #change icons to * to also get icons-dark
        file(GLOB_RECURSE paths ${_BREEZEICONS_DIR}/icons/*/*/${icon}.svg )

        #We cannot copy symlinks unless the target is there, so copy non-symlinks first
        foreach(path ${paths})
            file(RELATIVE_PATH _rel_path ${_BREEZEICONS_DIR} ${path})
            set(_target_path ${CMAKE_CURRENT_SOURCE_DIR}/breeze/${_rel_path})
            get_filename_component(_target_dir ${_target_path} DIRECTORY)
            if (NOT EXISTS ${_target_path} AND NOT IS_SYMLINK ${path})
                message("Copying icon ${path} to ${_target_dir}")
                file(COPY ${path} DESTINATION ${_target_dir})
            endif()
        endforeach()
        #Now copy symlinks
        foreach(path ${paths})
            file(RELATIVE_PATH _rel_path ${_BREEZEICONS_DIR} ${path})
            set(_target_path ${CMAKE_CURRENT_SOURCE_DIR}/breeze/${_rel_path})
            get_filename_component(_target_dir ${_target_path} DIRECTORY)
            if (NOT EXISTS ${_target_path} AND IS_SYMLINK ${path})
                message("Copying icon symlink ${path} to ${_target_dir}")
                file(COPY ${path} DESTINATION ${_target_dir})
            endif()
        endforeach()

    endfunction()

    #Fetch the icons
    if (NOT EXISTS ${_BREEZEICONS_DIR})
        message("Cloning into ${_BREEZEICONS_DIR}")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 git://anongit.kde.org/breeze-icons.git ${_BREEZEICONS_DIR})
    endif()

    #Copy the icons we want
    foreach(_iconName ${ARG_ICONS})
        message (STATUS ${_iconName})
        _copy_breeze_icon(${_iconName})
    endforeach()
    #Copy the license file
    file(COPY ${_BREEZEICONS_DIR}/COPYING-ICONS DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/breeze/)
endfunction()



# Run this function if you want to updated the icons
# Warning: some icon names are symlinks and you'll thus need to copy the actual icon as well. If copying fails with some weird errors that's typically the problem
if (FALSE)
#file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/breeze/)
copy_breeze_icons(ICONS
    application-menu
    dialog-cancel
    dialog-ok
    document-decrypt
    document-edit
    document-encrypt
    edit-delete
    edit-find
    edit-undo
    error
    folder
    im-user
    mail-mark-important
    mail-mark-unread-new
    mail-reply-sender
    network-disconnect
    view-refresh
)
endif()


#Install kube icons
ecm_install_icons(ICONS sc-apps-kube_icon.svg 256-apps-kube_icon.png DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_symbol.svg DESTINATION share/icons)
ecm_install_icons(ICONS sc-apps-kube_logo.svg DESTINATION share/icons)

install(DIRECTORY
    breeze/icons/places
    breeze/icons/actions
    DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
install(FILES breeze/icons/index.theme DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/kube)
