set(mimetreeparser_SRCS
    interface.cpp
    objecttreesource.cpp
    stringhtmlwriter.cpp
)

add_library(mimetreeparser SHARED ${mimetreeparser_SRCS})

qt5_use_modules(mimetreeparser Core Gui)
target_link_libraries(mimetreeparser KF5::Mime KF5::MimeTreeParser)

install(TARGETS mimetreeparser
   DESTINATION ${LIB_INSTALL_DIR})

add_subdirectory(tests)
