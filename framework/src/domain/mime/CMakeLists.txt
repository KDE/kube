include_directories(${GPGME_INCLUDE_DIRS})
add_library(mailcrypto STATIC
    mailcrypto.cpp
)
target_link_libraries(mailcrypto
    PUBLIC
    Qt5::Core
    KF5::Mime
    Gpgme::Gpgme
    sink
)
