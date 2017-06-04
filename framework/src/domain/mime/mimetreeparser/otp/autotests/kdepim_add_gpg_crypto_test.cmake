# Copyright (c) 2013 Sandro Knauß <mail@sandroknauss.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set( MIMETREEPARSERRELPATH framework/src/domain/mime/mimetreeparser/otp)
set( GNUPGHOME ${CMAKE_BINARY_DIR}/${MIMETREEPARSERRELPATH}/tests/gnupg_home )
add_definitions( -DGNUPGHOME="${GNUPGHOME}" )

macro (ADD_GPG_CRYPTO_TEST _target _testname)
   if (UNIX)
      if (APPLE)
         set(_library_path_variable "DYLD_LIBRARY_PATH")
      elseif (CYGWIN)
         set(_library_path_variable "PATH")
      else (APPLE)
         set(_library_path_variable "LD_LIBRARY_PATH")
      endif (APPLE)

      if (APPLE)
         # DYLD_LIBRARY_PATH does not work like LD_LIBRARY_PATH
         # OSX already has the RPATH in libraries and executables, putting runtime directories in
         # DYLD_LIBRARY_PATH actually breaks things
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/")
      else (APPLE)
         set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/:${LIB_INSTALL_DIR}:${QT_LIBRARY_DIR}")
      endif (APPLE)
      set(_executable "$<TARGET_FILE:${_target}>")

      # use add_custom_target() to have the sh-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -D_filename=${_executable}.shell -D_library_path_variable=${_library_path_variable}
        -D_ld_library_path="${_ld_library_path}" -D_executable=$<TARGET_FILE:${_target}>
        -D_gnupghome="${GNUPGHOME}"
        -P ${CMAKE_SOURCE_DIR}/${MIMETREEPARSERRELPATH}/tests/kdepim_generate_crypto_test_wrapper.cmake
      )

      set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${_executable}.shell" )
      add_test(NAME ${_testname} COMMAND ${_executable}.shell)

   else (UNIX)
      # under windows, set the property WRAPPER_SCRIPT just to the name of the executable
      # maybe later this will change to a generated batch file (for setting the PATH so that the Qt libs are found)
      set(_ld_library_path "${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}\;${LIB_INSTALL_DIR}\;${QT_LIBRARY_DIR}")
      set(_executable "$<TARGET_FILE:${_target}>")

      # use add_custom_target() to have the batch-file-wrapper generated during build time instead of cmake time
      add_custom_command(TARGET ${_target} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         -D_filename="${_executable}.bat"
         -D_ld_library_path="${_ld_library_path}" -D_executable="${_executable}"
         -D_gnupghome="${GNUPGHOME}"
         -P ${CMAKE_SOURCE_DIR}/${MIMETREEPARSERRELPATH}/tests/kdepim_generate_crypto_test_wrapper.cmake
         )

      add_test(NAME ${_testname} COMMAND ${_executable}.bat)

   endif (UNIX)
endmacro (ADD_GPG_CRYPTO_TEST)

