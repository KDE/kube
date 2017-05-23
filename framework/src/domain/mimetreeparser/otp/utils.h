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

#ifndef __MIMETREEPARSER_BODYFORAMATTER_UTILS_H__
#define __MIMETREEPARSER_BODYFORAMATTER_UTILS_H__

#include "bodypart.h"
#include "messagepart.h"

#include <KMime/Content>

namespace MimeTreeParser
{
/**
  1. Create a new partNode using 'content' data and Content-Description
     found in 'cntDesc'.
  2. Parse the 'node' to display the content.
*/
MimeMessagePart::Ptr createAndParseTempNode(Interface::BodyPart &part, KMime::Content *parentNode, const char *content, const char *cntDesc);

KMime::Content *findTypeInDirectChilds(KMime::Content *content, const QByteArray &mimeType);

MessagePart::Ptr toplevelTextNode(MessagePart::Ptr messageTree);
}

#endif
