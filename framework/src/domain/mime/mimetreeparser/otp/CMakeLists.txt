add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions("-fvisibility=default")

find_package(Qt5 COMPONENTS REQUIRED Core Gui)
find_package(KF5Mime "4.87.0" CONFIG REQUIRED)
find_package(QGpgme CONFIG REQUIRED)
find_package(KF5Codecs CONFIG REQUIRED)
find_package(KF5I18n CONFIG REQUIRED)

#add_definitions(-DTRANSLATION_DOMAIN=\"libmimetreeparser\")

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
    objecttreesource.cpp
    bodypart.cpp

    #bodypartformatter.cpp
    bodypartformatterbasefactory.cpp
    cryptohelper.cpp
    nodehelper.cpp
    messagepart.cpp
    partnodebodypart.cpp
    #Mementos
    cryptobodypartmemento.cpp
    decryptverifybodypartmemento.cpp
    verifydetachedbodypartmemento.cpp
    verifyopaquebodypartmemento.cpp
    #Stuff
    mimetreeparser_debug.cpp
    qgpgmejobexecutor.cpp
    util.cpp
    attachmenttemporaryfilesdirs.cpp
    )

add_library(kube_otp ${libmimetreeparser_SRCS})

target_link_libraries(kube_otp
    PUBLIC
    KF5::Mime
)

target_link_libraries(kube_otp
    PRIVATE
    QGpgme
    KF5::Codecs
    KF5::I18n
    Qt5::Gui
)
install(TARGETS kube_otp DESTINATION ${LIB_INSTALL_DIR})

add_subdirectory(autotests)
