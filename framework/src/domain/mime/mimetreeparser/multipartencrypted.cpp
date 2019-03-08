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

#include "multipartencrypted.h"

#include "utils.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

#include "mimetreeparser_debug.h"

using namespace MimeTreeParser;

const MultiPartEncryptedBodyPartFormatter *MultiPartEncryptedBodyPartFormatter::self;

const Interface::BodyPartFormatter *MultiPartEncryptedBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartEncryptedBodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr MultiPartEncryptedBodyPartFormatter::process(Interface::BodyPart &part) const
{
    KMime::Content *node = part.content();

    if (node->contents().isEmpty()) {
        Q_ASSERT(false);
        return MessagePart::Ptr();
    }

    CryptoProtocol useThisCryptProto = UnknownProtocol;

    /*
    ATTENTION: This code is to be replaced by the new 'auto-detect' feature. --------------------------------------
    */
    KMime::Content *data = findTypeInDirectChildren(node, "application/octet-stream");
    if (data) {
        useThisCryptProto = OpenPGP;
    } else {
        data = findTypeInDirectChildren(node, "application/pkcs7-mime");
        if (data) {
            useThisCryptProto = CMS;
        }
    }
    /*
    ---------------------------------------------------------------------------------------------------------------
    */

    if (!data) {
        return MessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), node->contents().at(0)));
    }

    EncryptedMessagePart::Ptr mp(new EncryptedMessagePart(part.objectTreeParser(),
                                 data->decodedText(),
                                 useThisCryptProto,
                                 part.nodeHelper()->fromAsString(data),
                                 node,
                                 data));
    mp->setIsEncrypted(true);
    return mp;
}
