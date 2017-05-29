/*  -*- mode: C++; c-file-style: "gnu" -*-
    bodypartformatter.h

    This file is part of KMail's plugin interface.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>,
                       Ingo Kloecker <kloecker@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __MIMETREEPARSER_INTERFACE_BODYPARTFORMATTER_H__
#define __MIMETREEPARSER_INTERFACE_BODYPARTFORMATTER_H__

#include <QObject>
#include <QSharedPointer>

#include "objecttreeparser.h"

namespace MimeTreeParser
{
class HtmlWriter;

namespace Interface
{

class BodyPartURLHandler;
class BodyPart;
class MessagePartPrivate;

class MessagePart : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString plaintextContent READ plaintextContent)
    Q_PROPERTY(QString htmlContent READ htmlContent)
public:
    typedef QSharedPointer<MessagePart> Ptr;
    explicit MessagePart();
    explicit MessagePart(const BodyPart &part);
    virtual ~MessagePart();

    virtual void html(bool decorate);
    virtual QString text() const;

    void setParentPart(MessagePart *parentPart);
    MessagePart *parentPart() const;

    virtual QString plaintextContent() const;
    virtual QString htmlContent() const;

private:
    MessagePartPrivate *d;

    friend class BodyPartFormatter;
};

class BodyPartFormatter
{
public:
    virtual ~BodyPartFormatter() {}

    /**
    @li Ok returned when format() generated some HTML
    @li NeedContent returned when format() needs the body of the part
    @li AsIcon returned when the part should be shown iconified
    @li Failed returned when formatting failed. Currently equivalent to Ok
    */
    enum Result { Ok, NeedContent, AsIcon, Failed };

    /**
    Format body part \a part by generating some HTML and writing
    that to \a writer.

    @return the result code (see above)
    */
    virtual Result format(BodyPart *part, MimeTreeParser::HtmlWriter *writer) const = 0;

    /**
      Variant of format that allows implementors to hook notifications up to
      a listener interested in the result, for async operations.

      @return the result code (see above)
    */
    virtual Result format(BodyPart *part, MimeTreeParser::HtmlWriter *writer, QObject *asyncResultObserver) const
    {
        Q_UNUSED(asyncResultObserver);
        return format(part, writer);
    }

    virtual void adaptProcessResult(ProcessResult &result) const
    {
        Q_UNUSED(result);
    }
    virtual MessagePart::Ptr process(BodyPart &part) const;
};

/**
    @short interface for BodyPartFormatter plugins

    The interface is queried by for types, subtypes, and the
    corresponding bodypart formatter, and the result inserted into
    the bodypart formatter factory.

    Subtype alone or both type and subtype may be "*", which is
    taken as a wildcard, so that e.g. type=text subtype=* matches
    any text subtype, but with lesser specificity than a concrete
    mimetype such as text/plain. type=* is only allowed when
    subtype=*, too.
*/
class BodyPartFormatterPlugin
{
public:
    virtual ~BodyPartFormatterPlugin() {}

    virtual const BodyPartFormatter *bodyPartFormatter(int idx) const = 0;
    virtual const char *type(int idx) const = 0;
    virtual const char *subtype(int idx) const = 0;

    virtual const BodyPartURLHandler *urlHandler(int idx) const = 0;
};

} // namespace Interface

}
#endif // __MIMETREEPARSER_INTERFACE_BODYPARTFORMATTER_H__
