/*  -*- mode: C++; c-file-style: "gnu" -*-
    bodypart.h

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

#ifndef __MIMETREEPARSER_INTERFACES_BODYPART_H__
#define __MIMETREEPARSER_INTERFACES_BODYPART_H__

#include <QByteArray>
#include <QString>

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{
class NodeHelper;
class ObjectTreeParser;

namespace Interface
{


/**
    @short interface of classes that implement status for BodyPartFormatters.
*/
class BodyPartMemento
{
public:
    virtual ~BodyPartMemento();

    virtual void detach() = 0;
};

/**
    @short interface of message body parts.
*/
class BodyPart
{
public:
    virtual ~BodyPart();

    /**
    @return a string respresentation of an URL that can be used
    to invoke a BodyPartURLHandler for this body part.
    */
    virtual QString makeLink(const QString &path) const = 0;

    /**
    @return the decoded (CTE, canonicalisation, and charset
    encoding undone) text contained in the body part, or
    QString(), it the body part is not of type "text".
    */
    virtual QString asText() const = 0;

    /**
    @return the decoded (CTE undone) content of the body part, or
    a null array if this body part instance is of type text.
    */
    virtual QByteArray asBinary() const = 0;

    /**
    @return the value of the content-type header field parameter
    with name \a parameter, or QString(), if that that
    parameter is not present in the body's content-type header
    field. RFC 2231 encoding is removed first.

    Note that this method will suppress queries to certain
    standard parameters (most notably "charset") to keep plugins
    decent.

    Note2 that this method preserves the case of the parameter
    value returned. So, if the parameter you want to use defines
    the value to be case-insensitive (such as the smime-type
    parameter), you need to make sure you do the casemap yourself
    before comparing to a reference value.
    */
    virtual QString contentTypeParameter(const char *parameter) const = 0;

    /**
    @return the content of the content-description header field,
    or QString() if that header is not present in this body
    part. RFC 2047 encoding is decoded first.
    */
    virtual QString contentDescription() const = 0;

    //virtual int contentDisposition() const = 0;
    /**
    @return the value of the content-disposition header field
    parameter with name \a parameter, or QString() if that
    parameter is not present in the body's content-disposition
    header field. RFC 2231 encoding is removed first.

    The notes made for contentTypeParameter() above apply here as
    well.
    */
    virtual QString contentDispositionParameter(const char *parameter) const = 0;

    /** Returns the KMime::Content node represented here. Makes most of the above obsolete
        and probably should be used in the interfaces in the first place.
    */
    virtual KMime::Content *content() const = 0;

    /**
     * Returns the top-level content.
     * Note that this is _not_ necessarily the same as content()->topLevel(), for example the later
     * will not work for "extra nodes", i.e. nodes in encrypted parts of the mail.
     * topLevelContent() will return the correct result in this case. Also note that
     * topLevelContent()
     */
    virtual KMime::Content *topLevelContent() const = 0;

    /**
     * Ok, this is ugly, exposing the node helper here, but there is too much useful stuff in there
     * for real-world plugins. Still, there should be a nicer way for this.
     */
    virtual MimeTreeParser::NodeHelper *nodeHelper() const = 0;

    /**
     * For making it easier to refactor, add objectTreeParser
     */
    virtual MimeTreeParser::ObjectTreeParser *objectTreeParser() const = 0;
};

} // namespace Interface

}

#endif // __MIMETREEPARSER_INTERFACES_BODYPART_H__
