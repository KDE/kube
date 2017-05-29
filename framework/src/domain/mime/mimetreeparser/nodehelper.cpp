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

#include "nodehelper.h"
#include "mimetreeparser_debug.h"
#include "partmetadata.h"
#include "bodypart.h"
#include "attachmenttemporaryfilesdirs.h"

#include <KMime/Content>
#include <KMime/Message>
#include <KMime/Headers>

#include <QTemporaryFile>
#include <KLocalizedString>
#include <kcharsets.h>

#include <QUrl>
#include <QDir>
#include <QTextCodec>

#include <string>
#include <sstream>
#include <algorithm>
#include <KCharsets>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileDevice>

namespace MimeTreeParser
{

NodeHelper::NodeHelper() :
    mAttachmentFilesDir(new AttachmentTemporaryFilesDirs())
{
    //TODO(Andras) add methods to modify these prefixes

    mLocalCodec = QTextCodec::codecForLocale();

    // In the case of Japan. Japanese locale name is "eucjp" but
    // The Japanese mail systems normally used "iso-2022-jp" of locale name.
    // We want to change locale name from eucjp to iso-2022-jp at KMail only.

    // (Introduction to i18n, 6.6 Limit of Locale technology):
    // EUC-JP is the de-facto standard for UNIX systems, ISO 2022-JP
    // is the standard for Internet, and Shift-JIS is the encoding
    // for Windows and Macintosh.
    if (mLocalCodec) {
        const QByteArray codecNameLower = mLocalCodec->name().toLower();
        if (codecNameLower == "eucjp"
#if defined Q_OS_WIN || defined Q_OS_MACX
                || codecNameLower == "shift-jis" // OK?
#endif
           ) {
            mLocalCodec = QTextCodec::codecForName("jis7");
            // QTextCodec *cdc = QTextCodec::codecForName("jis7");
            // QTextCodec::setCodecForLocale(cdc);
            // KLocale::global()->setEncoding(cdc->mibEnum());
        }
    }
}

NodeHelper::~NodeHelper()
{
    if (mAttachmentFilesDir) {
        mAttachmentFilesDir->forceCleanTempFiles();
        delete mAttachmentFilesDir;
        mAttachmentFilesDir = nullptr;
    }
    clear();
}

void NodeHelper::setNodeProcessed(KMime::Content *node, bool recurse)
{
    if (!node) {
        return;
    }
    mProcessedNodes.append(node);
    qCDebug(MIMETREEPARSER_LOG) << "Node processed: " << node->index().toString() << node->contentType()->as7BitString();
    //<< " decodedContent" << node->decodedContent();
    if (recurse) {
        const auto contents = node->contents();
        for (KMime::Content *c : contents) {
            setNodeProcessed(c, true);
        }
    }
}

void NodeHelper::setNodeUnprocessed(KMime::Content *node, bool recurse)
{
    if (!node) {
        return;
    }
    mProcessedNodes.removeAll(node);

    //avoid double addition of extra nodes, eg. encrypted attachments
    const QMap<KMime::Content *, QList<KMime::Content *> >::iterator it = mExtraContents.find(node);
    if (it != mExtraContents.end()) {
        Q_FOREACH (KMime::Content *c, it.value()) {
            KMime::Content *p = c->parent();
            if (p) {
                p->removeContent(c);
            }
        }
        qDeleteAll(it.value());
        qCDebug(MIMETREEPARSER_LOG) << "mExtraContents deleted for" << it.key();
        mExtraContents.erase(it);
    }

    qCDebug(MIMETREEPARSER_LOG) << "Node UNprocessed: " << node;
    if (recurse) {
        const auto contents = node->contents();
        for (KMime::Content *c : contents) {
            setNodeUnprocessed(c, true);
        }
    }
}

bool NodeHelper::nodeProcessed(KMime::Content *node) const
{
    if (!node) {
        return true;
    }
    return mProcessedNodes.contains(node);
}

void NodeHelper::clear()
{
    mProcessedNodes.clear();
    mOverrideCodecs.clear();
    QMap<KMime::Content *, QList<KMime::Content *> >::ConstIterator end(mExtraContents.constEnd());

    for (QMap<KMime::Content *, QList<KMime::Content *> >::ConstIterator it = mExtraContents.constBegin(); it != end; ++it) {
        Q_FOREACH (KMime::Content *c, it.value()) {
            KMime::Content *p = c->parent();
            if (p) {
                p->removeContent(c);
            }
        }
        qDeleteAll(it.value());
        qCDebug(MIMETREEPARSER_LOG) << "mExtraContents deleted for" << it.key();
    }
    mExtraContents.clear();
}


PartMetaData NodeHelper::partMetaData(KMime::Content *node)
{
    return mPartMetaDatas.value(node, PartMetaData());
}

void NodeHelper::setPartMetaData(KMime::Content *node, const PartMetaData &metaData)
{
    mPartMetaDatas.insert(node, metaData);
}

void NodeHelper::forceCleanTempFiles()
{
    mAttachmentFilesDir->forceCleanTempFiles();
    delete mAttachmentFilesDir;
    mAttachmentFilesDir = nullptr;
}

void NodeHelper::removeTempFiles()
{
    //Don't delete it it will delete in class
    mAttachmentFilesDir->removeTempFiles();
    mAttachmentFilesDir = new AttachmentTemporaryFilesDirs();
}

void NodeHelper::addTempFile(const QString &file)
{
    mAttachmentFilesDir->addTempFile(file);
}

bool NodeHelper::isInEncapsulatedMessage(KMime::Content *node)
{
    const KMime::Content *const topLevel = node->topLevel();
    const KMime::Content *cur = node;
    while (cur && cur != topLevel) {
        const bool parentIsMessage = cur->parent() && cur->parent()->contentType(false) &&
                                     cur->parent()->contentType()->mimeType().toLower() == "message/rfc822";
        if (parentIsMessage && cur->parent() != topLevel) {
            return true;
        }
        cur = cur->parent();
    }
    return false;
}

QByteArray NodeHelper::charset(KMime::Content *node)
{
    if (node->contentType(false)) {
        return node->contentType(false)->charset();
    } else {
        return node->defaultCharset();
    }
}

void NodeHelper::magicSetType(KMime::Content *node, bool aAutoDecode)
{
    const QByteArray body = (aAutoDecode) ? node->decodedContent() : node->body();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForData(body);

    QString mimetype = mime.name();
    node->contentType()->setMimeType(mimetype.toLatin1());
}

void NodeHelper::setOverrideCodec(KMime::Content *node, const QTextCodec *codec)
{
    if (!node) {
        return;
    }

    mOverrideCodecs[node] = codec;
}

const QTextCodec *NodeHelper::codec(KMime::Content *node)
{
    if (! node) {
        return mLocalCodec;
    }

    const QTextCodec *c = mOverrideCodecs.value(node, nullptr);
    if (!c) {
        // no override-codec set for this message, try the CT charset parameter:
        QByteArray charset = node->contentType()->charset();

        // utf-8 is a superset of us-ascii, so we don't loose anything, if we it insead
        // utf-8 is nowadays that widely, that it is a good guess to use it to fix issus with broken clients.
        if (charset.toLower() == "us-ascii") {
            charset = "utf-8";
        }
        c = codecForName(charset);
    }
    if (!c) {
        // no charset means us-ascii (RFC 2045), so using local encoding should
        // be okay
        c = mLocalCodec;
    }
    return c;
}

const QTextCodec *NodeHelper::codecForName(const QByteArray &_str)
{
    if (_str.isEmpty()) {
        return nullptr;
    }
    QByteArray codec = _str.toLower();
    return KCharsets::charsets()->codecForName(QLatin1String(codec));
}

QString NodeHelper::fileName(const KMime::Content *node)
{
    QString name = const_cast<KMime::Content *>(node)->contentDisposition()->filename();
    if (name.isEmpty()) {
        name = const_cast<KMime::Content *>(node)->contentType()->name();
    }

    name = name.trimmed();
    return name;
}

/*!
  Creates a persistent index string that bridges the gap between the
  permanent nodes and the temporary ones.

  Used internally for robust indexing.
*/
QString NodeHelper::persistentIndex(const KMime::Content *node) const
{
    if (!node) {
        return QString();
    }

    QString indexStr = node->index().toString();
    if (indexStr.isEmpty()) {
        QMapIterator<KMime::Message::Content *, QList<KMime::Content *> > it(mExtraContents);
        while (it.hasNext()) {
            it.next();
            const auto &extraNodes = it.value();
            for (int i = 0; i < extraNodes.size(); i++) {
                if (extraNodes[i] == node) {
                    indexStr = QString::fromLatin1("e%1").arg(i);
                    const QString parentIndex = persistentIndex(it.key());
                    if (!parentIndex.isEmpty()) {
                        indexStr = QString::fromLatin1("%1:%2").arg(parentIndex, indexStr);
                    }
                    qWarning() << "Persistentindex: " << indexStr;
                    return indexStr;
                }
            }
        }
    } else {
        const KMime::Content *const topLevel = node->topLevel();
        //if the node is an extra node, prepend the index of the extra node to the url
        QMapIterator<KMime::Message::Content *, QList<KMime::Content *> > it(mExtraContents);
        while (it.hasNext()) {
            it.next();
            const QList<KMime::Content *> &extraNodes = extraContents(it.key());
            for (int i = 0; i < extraNodes.size(); ++i) {
                KMime::Content *const extraNode = extraNodes[i];
                if (topLevel == extraNode) {
                    indexStr.prepend(QStringLiteral("e%1:").arg(i));
                    const QString parentIndex = persistentIndex(it.key());
                    if (!parentIndex.isEmpty()) {
                        indexStr = QStringLiteral("%1:%2").arg(parentIndex, indexStr);
                    }
                    qWarning() << "Persistentindex: " << indexStr;
                    return indexStr;
                }
            }
        }
    }

    qWarning() << "Persistentindex: " << indexStr;
    return indexStr;
}

KMime::Content *NodeHelper::contentFromIndex(KMime::Content *node, const QString &persistentIndex) const
{
    KMime::Content *c = node->topLevel();
    if (c) {
        const QStringList pathParts = persistentIndex.split(QLatin1Char(':'), QString::SkipEmptyParts);
        const int pathPartsSize(pathParts.size());
        for (int i = 0; i < pathPartsSize; ++i) {
            const QString &path = pathParts[i];
            if (path.startsWith(QLatin1Char('e'))) {
                const QList<KMime::Content *> &extraParts = mExtraContents.value(c);
                const int idx = path.midRef(1, -1).toInt();
                c = (idx < extraParts.size()) ? extraParts[idx] : nullptr;
            } else {
                c = c->content(KMime::ContentIndex(path));
            }
            if (!c) {
                break;
            }
        }
    }
    return c;
}

QString NodeHelper::asHREF(const KMime::Content *node, const QString &place) const
{
    return QStringLiteral("attachment:%1?place=%2").arg(persistentIndex(node), place);
}

KMime::Content *NodeHelper::fromHREF(const KMime::Message::Ptr &mMessage, const QUrl &url) const
{
    if (url.isEmpty()) {
        return mMessage.data();
    }

    if (!url.isLocalFile()) {
        return contentFromIndex(mMessage.data(), url.adjusted(QUrl::StripTrailingSlash).path());
    } else {
        const QString path = url.toLocalFile();
        // extract from /<path>/qttestn28554.index.2.3:0:2/unnamed -> "2.3:0:2"
        // start of the index is something that is not a number followed by a dot: \D.
        // index is only made of numbers,"." and ":": ([0-9.:]+)
        // index is the last part of the folder name: /
        const QRegExp rIndex(QStringLiteral("\\D\\.([e0-9.:]+)/"));

        //search the occurence at most at the end
        if (rIndex.lastIndexIn(path) != -1) {
            return  contentFromIndex(mMessage.data(), rIndex.cap(1));
        }
        return mMessage.data();
    }
}

QString NodeHelper::fixEncoding(const QString &encoding)
{
    QString returnEncoding = encoding;
    // According to http://www.iana.org/assignments/character-sets, uppercase is
    // preferred in MIME headers
    const QString returnEncodingToUpper = returnEncoding.toUpper();
    if (returnEncodingToUpper.contains(QStringLiteral("ISO "))) {
        returnEncoding = returnEncodingToUpper;
        returnEncoding.replace(QLatin1String("ISO "), QStringLiteral("ISO-"));
    }
    return returnEncoding;
}

//-----------------------------------------------------------------------------
QString NodeHelper::encodingForName(const QString &descriptiveName)
{
    QString encoding = KCharsets::charsets()->encodingForName(descriptiveName);
    return NodeHelper::fixEncoding(encoding);
}

QStringList NodeHelper::supportedEncodings(bool usAscii)
{
    QStringList encodingNames = KCharsets::charsets()->availableEncodingNames();
    QStringList encodings;
    QMap<QString, bool> mimeNames;
    QStringList::ConstIterator constEnd(encodingNames.constEnd());
    for (QStringList::ConstIterator it = encodingNames.constBegin();
            it != constEnd; ++it) {
        QTextCodec *codec = KCharsets::charsets()->codecForName(*it);
        QString mimeName = (codec) ? QString::fromLatin1(codec->name()).toLower() : (*it);
        if (!mimeNames.contains(mimeName)) {
            encodings.append(KCharsets::charsets()->descriptionForEncoding(*it));
            mimeNames.insert(mimeName, true);
        }
    }
    encodings.sort();
    if (usAscii) {
        encodings.prepend(KCharsets::charsets()->descriptionForEncoding(QStringLiteral("us-ascii")));
    }
    return encodings;
}

QString NodeHelper::fromAsString(KMime::Content *node) const
{
    if (auto topLevel = dynamic_cast<KMime::Message *>(node->topLevel())) {
        return topLevel->from()->asUnicodeString();
    } else {
        auto realNode = std::find_if(mExtraContents.cbegin(), mExtraContents.cend(),
        [node](const QList<KMime::Content *> &nodes) {
            return nodes.contains(node);
        });
        if (realNode != mExtraContents.cend()) {
            return fromAsString(realNode.key());
        }
    }

    return QString();
}

void NodeHelper::attachExtraContent(KMime::Content *topLevelNode, KMime::Content *content)
{
    qCDebug(MIMETREEPARSER_LOG) << "mExtraContents added for" << topLevelNode << " extra content: " << content;
    mExtraContents[topLevelNode].append(content);
}

QList< KMime::Content * > NodeHelper::extraContents(KMime::Content *topLevelnode) const
{
    return mExtraContents.value(topLevelnode);
}

void NodeHelper::mergeExtraNodes(KMime::Content *node)
{
    if (!node) {
        return;
    }

    const QList<KMime::Content * > extraNodes = extraContents(node);
    for (KMime::Content *extra : extraNodes) {
        if (node->bodyIsMessage()) {
            qCWarning(MIMETREEPARSER_LOG) << "Asked to attach extra content to a kmime::message, this does not make sense. Attaching to:" << node <<
                                          node->encodedContent() << "\n====== with =======\n" <<  extra << extra->encodedContent();
            continue;
        }
        KMime::Content *c = new KMime::Content(node);
        c->setContent(extra->encodedContent());
        c->parse();
        node->addContent(c);
    }

    Q_FOREACH (KMime::Content *child, node->contents()) {
        mergeExtraNodes(child);
    }
}

void NodeHelper::cleanFromExtraNodes(KMime::Content *node)
{
    if (!node) {
        return;
    }
    const QList<KMime::Content * > extraNodes = extraContents(node);
    for (KMime::Content *extra : extraNodes) {
        QByteArray s = extra->encodedContent();
        const auto children = node->contents();
        for (KMime::Content *c : children) {
            if (c->encodedContent() == s) {
                node->removeContent(c);
            }
        }
    }
    Q_FOREACH (KMime::Content *child, node->contents()) {
        cleanFromExtraNodes(child);
    }
}

KMime::Message *NodeHelper::messageWithExtraContent(KMime::Content *topLevelNode)
{
    /*The merge is done in several steps:
      1) merge the extra nodes into topLevelNode
      2) copy the modified (merged) node tree into a new node tree
      3) restore the original node tree in topLevelNode by removing the extra nodes from it

      The reason is that extra nodes are assigned by pointer value to the nodes in the original tree.
    */
    if (!topLevelNode) {
        return nullptr;
    }

    mergeExtraNodes(topLevelNode);

    KMime::Message *m = new KMime::Message;
    m->setContent(topLevelNode->encodedContent());
    m->parse();

    cleanFromExtraNodes(topLevelNode);
//   qCDebug(MIMETREEPARSER_LOG) << "MESSAGE WITH EXTRA: " << m->encodedContent();
//   qCDebug(MIMETREEPARSER_LOG) << "MESSAGE WITHOUT EXTRA: " << topLevelNode->encodedContent();

    return m;
}

QVector<KMime::Content *> NodeHelper::attachmentsOfExtraContents() const
{
    QVector<KMime::Content *> result;
    for (auto it = mExtraContents.begin(); it != mExtraContents.end(); ++it) {
        foreach (auto content, it.value()) {
            if (KMime::isAttachment(content)) {
                result.push_back(content);
            } else {
                result += content->attachments();
            }
        }
    }
    return result;
}

}
