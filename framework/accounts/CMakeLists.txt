set(accountsplugin_SRCS
    accountsplugin.cpp
    accountfactory.cpp
    accountsmodel.cpp
)

add_library(accountsplugin SHARED ${accountsplugin_SRCS})

qt5_use_modules(accountsplugin Core Quick Qml)
target_link_libraries(accountsplugin sink mailplugin)

install(TARGETS accountsplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})
