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

#ifndef __MIMETREEPARSER_NODEHELPER_H__
#define __MIMETREEPARSER_NODEHELPER_H__

#include "partmetadata.h"
#include "enums.h"

#include <KMime/Message>

#include <QList>
#include <QMap>
#include <QSet>

class QUrl;
class QTextCodec;

namespace MimeTreeParser
{

/**
 * @author Andras Mantia <andras@kdab.net>
 */
class NodeHelper: public QObject
{
    Q_OBJECT
public:
    NodeHelper();

    ~NodeHelper();

    void setNodeProcessed(KMime::Content *node, bool recurse);
    void setNodeUnprocessed(KMime::Content *node, bool recurse);
    bool nodeProcessed(KMime::Content *node) const;
    void clear();
    void forceCleanTempFiles();

    void setPartMetaData(KMime::Content *node, const PartMetaData &metaData);
    PartMetaData partMetaData(KMime::Content *node);

    /**
     *  Set the 'Content-Type' by mime-magic from the contents of the body.
     *  If autoDecode is true the decoded body will be used for mime type
     *  determination (this does not change the body itself).
     */
    void magicSetType(KMime::Content *node, bool autoDecode = true);

    /** Get a QTextCodec suitable for this message part */
    const QTextCodec *codec(KMime::Content *node);

    /** Set the charset the user selected for the message to display */
    void setOverrideCodec(KMime::Content *node, const QTextCodec *codec);

    /**
     * @return true if this node is a child or an encapsulated message
     */
    static bool isInEncapsulatedMessage(KMime::Content *node);

    /**
     * Returns the charset for the given node. If no charset is specified
     * for the node, the defaultCharset() is returned.
     */
    static QByteArray charset(KMime::Content *node);

    /**
     * Return a QTextCodec for the specified charset.
     * This function is a bit more tolerant, than QTextCodec::codecForName
     */
    static const QTextCodec *codecForName(const QByteArray &_str);

    /**
     * Returns a usable filename for a node, that can be the filename from the
     * content disposition header, or if that one is empty, the name from the
     * content type header.
     */
    static QString fileName(const KMime::Content *node);

    /**
     * Fixes an encoding received by a KDE function and returns the proper,
     * MIME-compilant encoding name instead.
     * @see encodingForName
     */
    static QString fixEncoding(const QString &encoding);   //TODO(Andras) move to a utility class?

    /**
     * Drop-in replacement for KCharsets::encodingForName(). The problem with
     * the KCharsets function is that it returns "human-readable" encoding names
     * like "ISO 8859-15" instead of valid encoding names like "ISO-8859-15".
     * This function fixes this by replacing whitespace with a hyphen.
     */
    static QString encodingForName(const QString &descriptiveName);   //TODO(Andras) move to a utility class?

    /**
     * Return a list of the supported encodings
     * @param usAscii if true, US-Ascii encoding will be prepended to the list.
     */
    static QStringList supportedEncodings(bool usAscii);   //TODO(Andras) move to a utility class?

    QString fromAsString(KMime::Content *node) const;

Q_SIGNALS:
    void update(MimeTreeParser::UpdateMode);

private:
    Q_DISABLE_COPY(NodeHelper)
    bool unencryptedMessage_helper(KMime::Content *node, QByteArray &resultingData, bool addHeaders,
                                   int recursionLevel = 1);

private:
    QList<KMime::Content *> mProcessedNodes;
    QList<KMime::Content *> mNodesUnderProcess;
    QTextCodec *mLocalCodec;
    QMap<KMime::Content *, const QTextCodec *> mOverrideCodecs;
    QMap<KMime::Content *, PartMetaData> mPartMetaDatas;

    friend class NodeHelperTest;
};

}

#endif
