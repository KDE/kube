find_path(GPGME_INCLUDE_DIR NAMES gpgme.h)
find_path(GPGERROR_INCLUDE_DIR NAMES gpg-error.h)
find_library(GPGME_LIBRARY NAMES gpgme)
find_library(GPGERROR_LIBRARY NAMES gpg-error)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GPGME DEFAULT_MSG GPGME_INCLUDE_DIR GPGERROR_INCLUDE_DIR GPGME_LIBRARY GPGERROR_LIBRARY)

mark_as_advanced(GPGME_INCLUDE_DIR GPGME_LIBRARY GPGME_INCLUDE_DIR GPGME_LIBRARY)

set(GPGME_LIBRARIES ${GPGME_LIBRARY} ${GPGERROR_LIBRARY})
set(GPGME_INCLUDE_DIRS ${GPGME_INCLUDE_DIR} ${GPGERROR_INCLUDE_DIR})

if (NOT ${gpgme})
    add_library(gpgme INTERFACE)
    target_link_libraries(gpgme INTERFACE ${GPGME_LIBRARIES})
    target_include_directories(gpgme INTERFACE ${GPGME_INCLUDE_DIRS})
endif()
