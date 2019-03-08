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

#include "applicationpgpencrypted.h"

#include "utils.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

#include "mimetreeparser_debug.h"

using namespace MimeTreeParser;

const ApplicationPGPEncryptedBodyPartFormatter *ApplicationPGPEncryptedBodyPartFormatter::self;

const Interface::BodyPartFormatter *ApplicationPGPEncryptedBodyPartFormatter::create()
{
    if (!self) {
        self = new ApplicationPGPEncryptedBodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr ApplicationPGPEncryptedBodyPartFormatter::process(Interface::BodyPart &part) const
{
    KMime::Content *node(part.content());

    if (node->decodedContent().trimmed() != "Version: 1") {
        qCWarning(MIMETREEPARSER_LOG) << "Unknown PGP Version String:" << node->decodedContent().trimmed();
    }

    if (!part.content()->parent()) {
        return MessagePart::Ptr();
    }

    KMime::Content *data = findTypeInDirectChildren(part.content()->parent(), "application/octet-stream");

    if (!data) {
        return MessagePart::Ptr(); //new MimeMessagePart(part.objectTreeParser(), node));
    }

    EncryptedMessagePart::Ptr mp(new EncryptedMessagePart(part.objectTreeParser(),
                                 data->decodedText(), OpenPGP,
                                 part.nodeHelper()->fromAsString(data), node, data));
    mp->setIsEncrypted(true);
    return mp;
}
