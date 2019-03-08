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

#include "utils.h"
#include "nodehelper.h"

using namespace MimeTreeParser;

MimeMessagePart::Ptr MimeTreeParser::createAndParseTempNode(Interface::BodyPart &part, const char *content, const char *cntDesc)
{
    KMime::Content *newNode = new KMime::Content();
    newNode->setContent(KMime::CRLFtoLF(content));
    newNode->parse();

    if (!newNode->head().isEmpty()) {
        newNode->contentDescription()->from7BitString(cntDesc);
    }

    auto mp = MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), newNode));
    mp->bindLifetime(newNode);
    return mp;
}

KMime::Content *MimeTreeParser::findTypeInDirectChildren(KMime::Content *content, const QByteArray &mimeType)
{
    for (const auto child : content->contents()) {
        if ((!child->contentType()->isEmpty())
                && (mimeType == child->contentType()->mimeType())) {
            return child;
        }
    }
    return nullptr;
}

MessagePart::Ptr MimeTreeParser::toplevelTextNode(MessagePart::Ptr messageTree)
{
    foreach (const auto &mp, messageTree->subParts()) {
        auto text = mp.dynamicCast<TextMessagePart>();
        auto attach = mp.dynamicCast<AttachmentMessagePart>();
        if (text && !attach) {
            return text;
        } else if (const auto alternative = mp.dynamicCast<AlternativeMessagePart>()) {
            return alternative;
        } else if (const auto m = mp.dynamicCast<MessagePart>()) {
            auto ret = toplevelTextNode(m);
            if (ret) {
                return ret;
            }
        }
    }
    return MessagePart::Ptr();
}
