add_library(mailcrypto STATIC
    mailcrypto.cpp
)
target_link_libraries(mailcrypto
    Qt5::Core
    KF5::Mime
    QGpgme
    Gpgmepp
)
