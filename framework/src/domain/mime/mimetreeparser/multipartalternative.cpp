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

#include "multipartalternative.h"

#include "utils.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

#include "mimetreeparser_debug.h"

using namespace MimeTreeParser;

const MultiPartAlternativeBodyPartFormatter *MultiPartAlternativeBodyPartFormatter::self;

const Interface::BodyPartFormatter *MultiPartAlternativeBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartAlternativeBodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr MultiPartAlternativeBodyPartFormatter::process(Interface::BodyPart &part) const
{
    KMime::Content *node = part.content();
    if (node->contents().isEmpty()) {
        return MessagePart::Ptr();
    }

    //Hardcoded after removing the source
    auto preferredMode = MimeTreeParser::Util::Html;
    AlternativeMessagePart::Ptr mp(new AlternativeMessagePart(part.objectTreeParser(), node));
    if (mp->mChildNodes.isEmpty()) {
        return MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), node->contents().at(0), false));
    }

    KMime::Content *dataIcal = mp->mChildNodes.contains(Util::MultipartIcal) ? mp->mChildNodes[Util::MultipartIcal] : nullptr;
    KMime::Content *dataHtml = mp->mChildNodes.contains(Util::MultipartHtml) ? mp->mChildNodes[Util::MultipartHtml] : nullptr;
    KMime::Content *dataPlain = mp->mChildNodes.contains(Util::MultipartPlain) ? mp->mChildNodes[Util::MultipartPlain] : nullptr;

    // Make sure that in default ical is prefered over html and plain text
    if (dataIcal && ((preferredMode != Util::MultipartHtml && preferredMode != Util::MultipartPlain))) {
        if (dataHtml) {
            part.nodeHelper()->setNodeProcessed(dataHtml, false);
        }
        if (dataPlain) {
            part.nodeHelper()->setNodeProcessed(dataPlain, false);
        }
        preferredMode = Util::MultipartIcal;
    } else if ((dataHtml && (preferredMode == Util::MultipartHtml || preferredMode == Util::Html)) ||
               (dataHtml && dataPlain && dataPlain->body().isEmpty())) {
        if (dataPlain) {
            part.nodeHelper()->setNodeProcessed(dataPlain, false);
        }
        preferredMode = Util::MultipartHtml;
    } else if (!(preferredMode == Util::MultipartHtml) && dataPlain) {
        part.nodeHelper()->setNodeProcessed(dataHtml, false);
        preferredMode = Util::MultipartPlain;
    }
    return mp;
}
