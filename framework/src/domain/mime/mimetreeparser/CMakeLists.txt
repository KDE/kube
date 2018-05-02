set(CMAKE_CXX_VISIBILITY_PRESET default)

find_package(Qt5 COMPONENTS REQUIRED Core Gui)
find_package(KF5Mime 4.87.0 CONFIG REQUIRED)
find_package(QGpgme CONFIG REQUIRED)
find_package(Gpgmepp CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)

# target_include_directories does not handle empty include paths
include_directories(${GPGME_INCLUDES})

set(libmimetreeparser_SRCS
    objecttreeparser.cpp

    #Bodyformatter
    applicationpgpencrypted.cpp
    applicationpkcs7mime.cpp
    mailman.cpp
    multipartalternative.cpp
    multipartencrypted.cpp
    multipartmixed.cpp
    multipartsigned.cpp
    textplain.cpp
    texthtml.cpp
    utils.cpp
    bodypartformatter_impl.cpp

    #Interfaces
    bodypartformatter.cpp
    bodypart.cpp

    #bodypartformatter.cpp
    bodypartformatterbasefactory.cpp
    cryptohelper.cpp
    nodehelper.cpp
    messagepart.cpp
    partnodebodypart.cpp
    #Stuff
    mimetreeparser_debug.cpp
    util.cpp
    )

add_library(kube_otp STATIC ${libmimetreeparser_SRCS})

target_link_libraries(kube_otp
    PUBLIC
    KF5::Mime
)

target_link_libraries(kube_otp
    PRIVATE
    QGpgme
    Gpgmepp
    KF5::Codecs
    Qt5::Gui
)

add_subdirectory(autotests)
add_subdirectory(tests)
