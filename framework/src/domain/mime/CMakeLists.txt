add_library(mailcrypto STATIC
    mailcrypto.cpp
    crypto.cpp
)
target_link_libraries(mailcrypto
    PUBLIC
    Qt5::Core
    KF5::Mime
    Gpgme::Gpgme
)
