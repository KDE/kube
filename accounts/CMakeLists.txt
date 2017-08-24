include_directories(../framework/src/)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml)

find_package(Sink CONFIG REQUIRED)
find_package(KAsync CONFIG REQUIRED)
find_package(KF5 REQUIRED COMPONENTS Package Mime)

include_directories(SYSTEM ${KDE_INSTALL_FULL_INCLUDEDIR}/KF5/)

add_subdirectory(maildir)
add_subdirectory(imap)
add_subdirectory(kolabnow)
add_subdirectory(gmail)
