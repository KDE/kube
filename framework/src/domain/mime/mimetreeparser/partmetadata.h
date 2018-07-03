/*  -*- c++ -*-
    partmetadata.h

    KMail, the KDE mail client.
    Copyright (c) 2002-2003 Karl-Heinz Zimmer <khz@kde.org>
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __MIMETREEPARSER_PARTMETADATA_H__
#define __MIMETREEPARSER_PARTMETADATA_H__

#include <QStringList>
#include <QDateTime>

namespace MimeTreeParser
{

class PartMetaData
{
public:
    bool keyMissing = false;
    bool keyExpired = false;
    bool keyRevoked = false;
    bool sigExpired = false;
    bool crlMissing = false;
    bool crlTooOld = false;
    QString signer;
    QStringList signerMailAddresses;
    QByteArray keyId;
    bool keyIsTrusted = false;
    QString status;  // to be used for unknown plug-ins
    QString errorText;
    QDateTime creationTime;
    QString decryptionError;
    QString auditLog;
    bool isSigned = false;
    bool isGoodSignature =false;
    bool isEncrypted = false;
    bool isDecryptable = false;
    bool technicalProblem = false;
    bool isEncapsulatedRfc822Message = false;
};

}

#endif // __MIMETREEPARSER_PARTMETADATA_H__

