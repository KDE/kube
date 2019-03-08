/*
    objecttreeparser.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>
    Copyright (C) 2002-2003, 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

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

#ifndef __MIMETREEPARSER_OBJECTTREEPARSER_H__
#define __MIMETREEPARSER_OBJECTTREEPARSER_H__

#include "nodehelper.h"
#include "messagepart.h"

#include <functional>

class QString;

namespace KMime
{
class Content;
}

namespace MimeTreeParser
{

class PartMetaData;
class ViewerPrivate;
class NodeHelper;
class MimeMessagePart;

typedef QSharedPointer<MessagePart> MessagePartPtr;
typedef QSharedPointer<MimeMessagePart> MimeMessagePartPtr;

/**
 Entry point to parse mime messages.
*/
class ObjectTreeParser
{
    //Disable copy
    ObjectTreeParser(const ObjectTreeParser &other);
public:
    explicit ObjectTreeParser();

    explicit ObjectTreeParser(NodeHelper *nodeHelper);

    virtual ~ObjectTreeParser();

    QString structureAsString() const;
    void print();

    /**
    * The text of the message, ie. what would appear in the
    * composer's text editor if this was edited or replied to.
    * This is usually the content of the first text/plain MIME part.
    */
    QString plainTextContent();

    /**
    * Similar to plainTextContent(), but returns the HTML source of the first text/html MIME part.
    */
    QString htmlContent();

    NodeHelper *nodeHelper() const;

    /** Parse beginning at a given node and recursively parsing
      the children of that node and it's next sibling. */
    void parseObjectTree(KMime::Content *node);
    void parseObjectTree(const QByteArray &mimeMessage);
    MessagePartPtr parsedPart() const;
    KMime::Content *find(const std::function<bool(KMime::Content *)> &select);
    QVector<MessagePartPtr> collectContentParts();
    QVector<MessagePartPtr> collectContentParts(MessagePart::Ptr start);
    QVector<MessagePartPtr> collectAttachmentParts();

    /** Decrypt parts and verify signatures */
    void decrypt();
    void verifySignatures();
    void decryptAndVerify();
    //DEPRECATED calls decryptAndVerify
    void decryptParts();

    /** Import any certificates found in the message */
    void importCertificates();

    /** Embedd content referenced by cid by inlining */
    QString resolveCidLinks(const QString &html);

private:
    /**
    * Does the actual work for parseObjectTree. Unlike parseObjectTree(), this does not change the
    * top-level content.
    */
    MessagePartPtr parseObjectTreeInternal(KMime::Content *node, bool mOnlyOneMimePart);
    QVector<MessagePartPtr> processType(KMime::Content *node, const QByteArray &mediaType, const QByteArray &subType);

    QVector<MessagePartPtr> defaultHandling(KMime::Content *node);

private:

    /** ctor helper */
    void init();

    const QTextCodec *codecFor(KMime::Content *node) const;
private:
    NodeHelper *mNodeHelper;
    QByteArray mPlainTextContentCharset;
    QByteArray mHtmlContentCharset;
    QString mPlainTextContent;
    QString mHtmlContent;
    KMime::Content *mTopLevelContent;
    MessagePartPtr mParsedPart;

    bool mDeleteNodeHelper;
    KMime::Message::Ptr mMsg;

    friend class PartNodeBodyPart;
    friend class MessagePart;
    friend class EncryptedMessagePart;
    friend class SignedMessagePart;
    friend class EncapsulatedRfc822MessagePart;
    friend class TextMessagePart;
    friend class HtmlMessagePart;
    friend class TextPlainBodyPartFormatter;
    friend class MultiPartSignedBodyPartFormatter;
    friend class ApplicationPkcs7MimeBodyPartFormatter;
};

}

#endif // __MIMETREEPARSER_OBJECTTREEPARSER_H__

