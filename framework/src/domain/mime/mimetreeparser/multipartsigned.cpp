/*
   Copyright (c) 2016 Sandro Knauß <sknauss@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "multipartsigned.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

#include "mimetreeparser_debug.h"

#include <QTextCodec>

using namespace MimeTreeParser;

const MultiPartSignedBodyPartFormatter *MultiPartSignedBodyPartFormatter::self;

const Interface::BodyPartFormatter *MultiPartSignedBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartSignedBodyPartFormatter();
    }
    return self;
}
MessagePart::Ptr MultiPartSignedBodyPartFormatter::process(Interface::BodyPart &part) const
{
    KMime::Content *node = part.content();
    if (node->contents().size() != 2) {
        qCDebug(MIMETREEPARSER_LOG) << "mulitpart/signed must have exactly two child parts!" << endl
                                    << "processing as multipart/mixed";
        if (!node->contents().isEmpty()) {
            return MessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), node->contents().at(0)));
        } else {
            return MessagePart::Ptr();
        }
    }

    KMime::Content *signedData =  node->contents().at(0);
    KMime::Content *signature = node->contents().at(1);
    Q_ASSERT(signedData);
    Q_ASSERT(signature);

    QString protocolContentType = node->contentType()->parameter(QStringLiteral("protocol")).toLower();
    const QString signatureContentType = QLatin1String(signature->contentType()->mimeType().toLower());
    if (protocolContentType.isEmpty()) {
        qCWarning(MIMETREEPARSER_LOG) << "Message doesn't set the protocol for the multipart/signed content-type, "
                                      "using content-type of the signature:" << signatureContentType;
        protocolContentType = signatureContentType;
    }

    CryptoProtocol protocol = UnknownProtocol;
    if (protocolContentType == QLatin1String("application/pkcs7-signature") ||
            protocolContentType == QLatin1String("application/x-pkcs7-signature")) {
        protocol = CMS;
    } else if (protocolContentType == QLatin1String("application/pgp-signature") ||
               protocolContentType == QLatin1String("application/x-pgp-signature")) {
        protocol = OpenPGP;
    }

    if (protocol == UnknownProtocol) {
        return MessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), signedData));
    }

    part.nodeHelper()->setNodeProcessed(signature, true);

    const QByteArray cleartext = KMime::LFtoCRLF(signedData->encodedContent());
    const QTextCodec *aCodec(part.objectTreeParser()->codecFor(signedData));

    SignedMessagePart::Ptr mp(new SignedMessagePart(part.objectTreeParser(),
                              aCodec->toUnicode(cleartext), protocol,
                              part.nodeHelper()->fromAsString(node), signature, signedData));

    return mp;
}
