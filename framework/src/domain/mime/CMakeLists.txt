add_library(mailcrypto STATIC
    mailcrypto.cpp
    crypto.cpp
)
target_link_libraries(mailcrypto
    Qt5::Core
    KF5::Mime
    ${GPGME_LIBRARIES}
)
