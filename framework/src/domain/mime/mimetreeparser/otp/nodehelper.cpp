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

static void clearBodyPartMemento(QMap<QByteArray, Interface::BodyPartMemento *> &bodyPartMementoMap)
{
    for (QMap<QByteArray, Interface::BodyPartMemento *>::iterator
            it = bodyPartMementoMap.begin(), end = bodyPartMementoMap.end();
            it != end; ++it) {
        Interface::BodyPartMemento *memento = it.value();
        memento->detach();
        delete memento;
    }
    bodyPartMementoMap.clear();
}

void NodeHelper::clear()
{
    mProcessedNodes.clear();
    mEncryptionState.clear();
    mSignatureState.clear();
    mOverrideCodecs.clear();
    std::for_each(mBodyPartMementoMap.begin(), mBodyPartMementoMap.end(),
                  &clearBodyPartMemento);
    mBodyPartMementoMap.clear();
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

void NodeHelper::setEncryptionState(const KMime::Content *node, const KMMsgEncryptionState state)
{
    mEncryptionState[node] = state;
}

KMMsgEncryptionState NodeHelper::encryptionState(const KMime::Content *node) const
{
    return mEncryptionState.value(node, KMMsgNotEncrypted);
}

void NodeHelper::setSignatureState(const KMime::Content *node, const KMMsgSignatureState state)
{
    mSignatureState[node] = state;
}

KMMsgSignatureState NodeHelper::signatureState(const KMime::Content *node) const
{
    return mSignatureState.value(node, KMMsgNotSigned);
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

KMMsgEncryptionState NodeHelper::overallEncryptionState(KMime::Content *node) const
{
    KMMsgEncryptionState myState = KMMsgEncryptionStateUnknown;
    if (!node) {
        return myState;
    }

    KMime::Content *parent = node->parent();
    auto contents = parent ?  parent->contents() : KMime::Content::List();
    if (contents.isEmpty()) {
        contents.append(node);
    }
    int i = contents.indexOf(const_cast<KMime::Content *>(node));
    for (; i < contents.size(); ++i) {
        auto next = contents.at(i);
        KMMsgEncryptionState otherState = encryptionState(next);

        // NOTE: children are tested ONLY when parent is not encrypted
        if (otherState == KMMsgNotEncrypted && !next->contents().isEmpty()) {
            otherState = overallEncryptionState(next->contents().at(0));
        }

        if (otherState == KMMsgNotEncrypted && !extraContents(next).isEmpty()) {
            otherState = overallEncryptionState(extraContents(next).at(0));
        }

        if (next == node) {
            myState = otherState;
        }

        switch (otherState) {
        case KMMsgEncryptionStateUnknown:
            break;
        case KMMsgNotEncrypted:
            if (myState == KMMsgFullyEncrypted) {
                myState = KMMsgPartiallyEncrypted;
            } else if (myState != KMMsgPartiallyEncrypted) {
                myState = KMMsgNotEncrypted;
            }
            break;
        case KMMsgPartiallyEncrypted:
            myState = KMMsgPartiallyEncrypted;
            break;
        case KMMsgFullyEncrypted:
            if (myState != KMMsgFullyEncrypted) {
                myState = KMMsgPartiallyEncrypted;
            }
            break;
        case KMMsgEncryptionProblematic:
            break;
        }
    }

    qCDebug(MIMETREEPARSER_LOG) << "\n\n  KMMsgEncryptionState:" << myState;

    return myState;
}

KMMsgSignatureState NodeHelper::overallSignatureState(KMime::Content *node) const
{
    KMMsgSignatureState myState = KMMsgSignatureStateUnknown;
    if (!node) {
        return myState;
    }

    KMime::Content *parent = node->parent();
    auto contents = parent ? parent->contents() : KMime::Content::List();
    if (contents.isEmpty()) {
        contents.append(node);
    }
    int i = contents.indexOf(const_cast<KMime::Content *>(node));
    for (; i < contents.size(); ++i) {
        auto next = contents.at(i);
        KMMsgSignatureState otherState = signatureState(next);

        // NOTE: children are tested ONLY when parent is not encrypted
        if (otherState == KMMsgNotSigned && !next->contents().isEmpty()) {
            otherState = overallSignatureState(next->contents().at(0));
        }

        if (otherState == KMMsgNotSigned && !extraContents(next).isEmpty()) {
            otherState = overallSignatureState(extraContents(next).at(0));
        }

        if (next == node) {
            myState = otherState;
        }

        switch (otherState) {
        case KMMsgSignatureStateUnknown:
            break;
        case KMMsgNotSigned:
            if (myState == KMMsgFullySigned) {
                myState = KMMsgPartiallySigned;
            } else if (myState != KMMsgPartiallySigned) {
                myState = KMMsgNotSigned;
            }
            break;
        case KMMsgPartiallySigned:
            myState = KMMsgPartiallySigned;
            break;
        case KMMsgFullySigned:
            if (myState != KMMsgFullySigned) {
                myState = KMMsgPartiallySigned;
            }
            break;
        case KMMsgSignatureProblematic:
            break;
        }
    }

    qCDebug(MIMETREEPARSER_LOG) << "\n\n  KMMsgSignatureState:" << myState;

    return myState;
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

//FIXME(Andras) review it (by Marc?) to see if I got it right. This is supposed to be the partNode::internalBodyPartMemento replacement
Interface::BodyPartMemento *NodeHelper::bodyPartMemento(KMime::Content *node,
        const QByteArray &which) const
{
    const QMap< QString, QMap<QByteArray, Interface::BodyPartMemento *> >::const_iterator nit
        = mBodyPartMementoMap.find(persistentIndex(node));
    if (nit == mBodyPartMementoMap.end()) {
        return nullptr;
    }
    const QMap<QByteArray, Interface::BodyPartMemento *>::const_iterator it =
        nit->find(which.toLower());
    if (it != nit->end()) {
        qWarning() << "Cache hit!!!!!!!!!!!!!!!!!";
        Q_ASSERT(false);
        return it.value();
    }
    return nullptr;
}

//FIXME(Andras) review it (by Marc?) to see if I got it right. This is supposed to be the partNode::internalSetBodyPartMemento replacement
void NodeHelper::setBodyPartMemento(KMime::Content *node, const QByteArray &which,
                                    Interface::BodyPartMemento *memento)
{
    QMap<QByteArray, Interface::BodyPartMemento *> &mementos
        = mBodyPartMementoMap[persistentIndex(node)];

    const QByteArray whichLower = which.toLower();
    const QMap<QByteArray, Interface::BodyPartMemento *>::iterator it =
        mementos.lowerBound(whichLower);

    if (it != mementos.end() && it.key() == whichLower) {
        delete it.value();
        if (memento) {
            it.value() = memento;
        } else {
            mementos.erase(it);
        }
    } else {
        mementos.insert(whichLower, memento);
    }
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

KMime::Content *NodeHelper::decryptedNodeForContent(KMime::Content *content) const
{
    const QList<KMime::Content *> xc = extraContents(content);
    if (!xc.empty()) {
        if (xc.size() == 1) {
            return xc.front();
        } else {
            qCWarning(MIMETREEPARSER_LOG) << "WTF, encrypted node has multiple extra contents?";
        }
    }
    return nullptr;
}

bool NodeHelper::unencryptedMessage_helper(KMime::Content *node, QByteArray &resultingData, bool addHeaders,
        int recursionLevel)
{
    bool returnValue = false;
    if (node) {
        KMime::Content *curNode = node;
        KMime::Content *decryptedNode = nullptr;
        const QByteArray type = node->contentType(false) ? QByteArray(node->contentType()->mediaType()).toLower() : "text";
        const QByteArray subType = node->contentType(false) ? node->contentType()->subType().toLower() : "plain";
        const bool isMultipart = node->contentType(false) && node->contentType()->isMultipart();
        bool isSignature = false;

        qCDebug(MIMETREEPARSER_LOG) << "(" << recursionLevel << ") Looking at" << type << "/" << subType;

        if (isMultipart) {
            if (subType == "signed") {
                isSignature = true;
            } else if (subType == "encrypted") {
                decryptedNode = decryptedNodeForContent(curNode);
            }
        } else if (type == "application") {
            if (subType == "octet-stream") {
                decryptedNode = decryptedNodeForContent(curNode);
            } else if (subType == "pkcs7-signature") {
                isSignature = true;
            } else if (subType == "pkcs7-mime") {
                // note: subtype pkcs7-mime can also be signed
                //       and we do NOT want to remove the signature!
                if (encryptionState(curNode) != KMMsgNotEncrypted) {
                    decryptedNode = decryptedNodeForContent(curNode);
                }
            }
        }

        if (decryptedNode) {
            qCDebug(MIMETREEPARSER_LOG) << "Current node has an associated decrypted node, adding a modified header "
                                        "and then processing the children.";

            Q_ASSERT(addHeaders);
            KMime::Content headers;
            headers.setHead(curNode->head());
            headers.parse();
            if (decryptedNode->contentType(false)) {
                headers.contentType()->from7BitString(decryptedNode->contentType()->as7BitString(false));
            } else {
                headers.removeHeader<KMime::Headers::ContentType>();
            }
            if (decryptedNode->contentTransferEncoding(false)) {
                headers.contentTransferEncoding()->from7BitString(decryptedNode->contentTransferEncoding()->as7BitString(false));
            } else {
                headers.removeHeader<KMime::Headers::ContentTransferEncoding>();
            }
            if (decryptedNode->contentDisposition(false)) {
                headers.contentDisposition()->from7BitString(decryptedNode->contentDisposition()->as7BitString(false));
            } else {
                headers.removeHeader<KMime::Headers::ContentDisposition>();
            }
            if (decryptedNode->contentDescription(false)) {
                headers.contentDescription()->from7BitString(decryptedNode->contentDescription()->as7BitString(false));
            } else {
                headers.removeHeader<KMime::Headers::ContentDescription>();
            }
            headers.assemble();

            resultingData += headers.head() + '\n';
            unencryptedMessage_helper(decryptedNode, resultingData, false, recursionLevel + 1);

            returnValue = true;
        }

        else if (isSignature) {
            qCDebug(MIMETREEPARSER_LOG) << "Current node is a signature, adding it as-is.";
            // We can't change the nodes under the signature, as that would invalidate it. Add the signature
            // and its child as-is
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }
            resultingData += curNode->encodedBody();
            returnValue = false;
        }

        else if (isMultipart) {
            qCDebug(MIMETREEPARSER_LOG) << "Current node is a multipart node, adding its header and then processing all children.";
            // Normal multipart node, add the header and all of its children
            bool somethingChanged = false;
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }
            const QByteArray boundary = curNode->contentType()->boundary();
            foreach (KMime::Content *child, curNode->contents()) {
                resultingData += "\n--" + boundary + '\n';
                const bool changed = unencryptedMessage_helper(child, resultingData, true, recursionLevel + 1);
                if (changed) {
                    somethingChanged = true;
                }
            }
            resultingData += "\n--" + boundary + "--\n\n";
            returnValue = somethingChanged;
        }

        else if (curNode->bodyIsMessage()) {
            qCDebug(MIMETREEPARSER_LOG) << "Current node is a message, adding the header and then processing the child.";
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }

            returnValue = unencryptedMessage_helper(curNode->bodyAsMessage().data(), resultingData, true, recursionLevel + 1);
        }

        else {
            qCDebug(MIMETREEPARSER_LOG) << "Current node is an ordinary leaf node, adding it as-is.";
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }
            resultingData += curNode->body();
            returnValue = false;
        }
    }

    qCDebug(MIMETREEPARSER_LOG) << "(" << recursionLevel << ") done.";
    return returnValue;
}

KMime::Message::Ptr NodeHelper::unencryptedMessage(const KMime::Message::Ptr &originalMessage)
{
    QByteArray resultingData;
    const bool messageChanged = unencryptedMessage_helper(originalMessage.data(), resultingData, true);
    if (messageChanged) {
#if 0
        qCDebug(MIMETREEPARSER_LOG) << "Resulting data is:" << resultingData;
        QFile bla("stripped.mbox");
        bla.open(QIODevice::WriteOnly);
        bla.write(resultingData);
        bla.close();
#endif
        KMime::Message::Ptr newMessage(new KMime::Message);
        newMessage->setContent(resultingData);
        newMessage->parse();
        return newMessage;
    } else {
        return KMime::Message::Ptr();
    }
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
