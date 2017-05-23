include_directories(.)

set(mimetreeparser_SRCS
    interface.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
)

add_library(mimetreeparser SHARED ${mimetreeparser_SRCS})

qt5_use_modules(mimetreeparser Core Gui)
target_link_libraries(mimetreeparser KF5::Mime kube_otp)

install(TARGETS mimetreeparser DESTINATION ${LIB_INSTALL_DIR})

add_subdirectory(tests)
add_subdirectory(otp)
