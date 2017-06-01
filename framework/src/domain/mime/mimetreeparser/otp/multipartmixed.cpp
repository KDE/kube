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

#include "multipartmixed.h"

#include "objecttreeparser.h"
#include "messagepart.h"

#include <KMime/Content>

#include "mimetreeparser_debug.h"

using namespace MimeTreeParser;

const MultiPartMixedBodyPartFormatter *MultiPartMixedBodyPartFormatter::self;

const Interface::BodyPartFormatter *MultiPartMixedBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartMixedBodyPartFormatter();
    }
    return self;
}
MessagePart::Ptr MultiPartMixedBodyPartFormatter::process(Interface::BodyPart &part) const
{
    if (part.content()->contents().isEmpty()) {
        return MessagePart::Ptr();
    }

    // normal treatment of the parts in the mp/mixed container
    return MimeMessagePart::Ptr(new MimeMessagePart(part.objectTreeParser(), part.content()->contents().at(0), false));
}
