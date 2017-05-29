/*
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __MIMETREEPARSER_OBJECTTREESOURCE_IF_H__
#define __MIMETREEPARSER_OBJECTTREESOURCE_IF_H__

#include "util.h"

#include <KMime/Message>

#include <QSharedPointer>
class QTextCodec;

namespace MimeTreeParser
{
class HtmlWriter;
class AttachmentStrategy;
class BodyPartFormatterBaseFactory;
namespace Interface
{
class MessagePart;
typedef QSharedPointer<MessagePart> MessagePartPtr;
}
}

namespace MimeTreeParser
{
namespace Interface
{

/**
 * Interface for object tree sources.
 * @author Andras Mantia <amantia@kdab.net>
 */
class ObjectTreeSource
{

public:
    virtual ~ObjectTreeSource();

    /**
      * Sets the type of mail that is currently displayed. Applications can display this
      * information to the user, for example KMail displays a HTML status bar.
      * Note: This is not called when the mode is "Normal".
      */
    virtual void setHtmlMode(MimeTreeParser::Util::HtmlMode mode, const QList<MimeTreeParser::Util::HtmlMode> &availableModes) = 0;

    /** Return the mode that is the preferred to display */
    virtual MimeTreeParser::Util::HtmlMode preferredMode() const = 0;

    /** Return true if an encrypted mail should be decrypted */
    virtual bool decryptMessage() const = 0;

    virtual int levelQuote() const = 0;

    /** The override codec that should be used for the mail */
    virtual const QTextCodec *overrideCodec() = 0;

    virtual QString createMessageHeader(KMime::Message *message) = 0;

    /** Return the wanted attachment startegy */
    virtual const AttachmentStrategy *attachmentStrategy() = 0;

    /** Return the html write object */

    /** The source object behind the interface. */
    virtual QObject *sourceObject() = 0;

    /** should keys be imported automatically **/
    virtual bool autoImportKeys() const = 0;

    virtual const BodyPartFormatterBaseFactory *bodyPartFormatterFactory() = 0;
};
}
}
#endif
