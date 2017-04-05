set(SRCS
    actionplugin.cpp
    action.cpp
    actionhandler.cpp
    actionbroker.cpp
    actionresult.cpp
    context.cpp
)

add_library(actionplugin SHARED ${SRCS})

target_link_libraries(actionplugin KAsync sink)
qt5_use_modules(actionplugin Core Quick Qml)

install(TARGETS actionplugin DESTINATION ${FRAMEWORK_INSTALL_DIR})

add_subdirectory(tests)
