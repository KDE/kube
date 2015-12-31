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
#include "messageviewer_debug.h"
#include "iconnamecache.h"
#include "settings/messageviewersettings.h"
#include "partmetadata.h"
#include "interfaces/bodypart.h"
#include "messageviewerutil.h"
// #include "PimCommon/AttachmentTemporaryFilesDirs"

#include <messagecore/nodehelper.h>
#include <messagecore/stringutil.h>
// #include "MessageCore/MessageCoreSettings"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_headers.h>

#include <QTemporaryFile>
#include <KLocalizedString>
#include <kcharsets.h>
// #include <kde_file.h>

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

namespace MessageViewer
{

QStringList replySubjPrefixes(QStringList() << QStringLiteral("Re\\s*:") << QStringLiteral("Re\\[\\d+\\]:") << QStringLiteral("Re\\d+:"));
QStringList forwardSubjPrefixes(QStringList() << QStringLiteral("Fwd:") << QStringLiteral("FW:"));

NodeHelper::NodeHelper()
    //: mAttachmentFilesDir(new PimCommon::AttachmentTemporaryFilesDirs())
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
        if (mLocalCodec->name().toLower() == "eucjp"
#if defined Q_OS_WIN || defined Q_OS_MACX
                || mLocalCodec->name().toLower() == "shift-jis" // OK?
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
    //Don't delete it it will delete in class with a deleteLater;
    // if (mAttachmentFilesDir) {
    //     mAttachmentFilesDir->removeTempFiles();
    //     mAttachmentFilesDir = 0;
    // }
}

void NodeHelper::setNodeProcessed(KMime::Content *node, bool recurse)
{
    if (!node) {
        return;
    }
    mProcessedNodes.append(node);
    qCDebug(MESSAGEVIEWER_LOG) << "Node processed: " << node->index().toString() << node->contentType()->as7BitString();
    //<< " decodedContent" << node->decodedContent();
    if (recurse) {
        auto contents = node->contents();
        Q_FOREACH (KMime::Content *c, contents) {
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
        qCDebug(MESSAGEVIEWER_LOG) << "mExtraContents deleted for" << it.key();
        mExtraContents.erase(it);
    }

    qCDebug(MESSAGEVIEWER_LOG) << "Node UNprocessed: " << node;
    if (recurse) {
        auto contents = node->contents();
        Q_FOREACH (KMime::Content *c, contents) {
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
        qCDebug(MESSAGEVIEWER_LOG) << "mExtraContents deleted for" << it.key();
    }
    mExtraContents.clear();
    mDisplayEmbeddedNodes.clear();
    mDisplayHiddenNodes.clear();
}

void NodeHelper::setEncryptionState(KMime::Content *node, const KMMsgEncryptionState state)
{
    mEncryptionState[node] = state;
}

KMMsgEncryptionState NodeHelper::encryptionState(KMime::Content *node) const
{
    return mEncryptionState.value(node, KMMsgNotEncrypted);
}

void NodeHelper::setSignatureState(KMime::Content *node, const KMMsgSignatureState state)
{
    mSignatureState[node] = state;
}

KMMsgSignatureState NodeHelper::signatureState(KMime::Content *node) const
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

QString NodeHelper::writeNodeToTempFile(KMime::Content *node)
{
    // If the message part is already written to a file, no point in doing it again.
    // This function is called twice actually, once from the rendering of the attachment
    // in the body and once for the header.
    QUrl existingFileName = tempFileUrlFromNode(node);
    if (!existingFileName.isEmpty()) {
        return existingFileName.toLocalFile();
    }

    QString fname = createTempDir(persistentIndex(node));
    if (fname.isEmpty()) {
        return QString();
    }

    QString fileName = NodeHelper::fileName(node);
    // strip off a leading path
    int slashPos = fileName.lastIndexOf(QLatin1Char('/'));
    if (-1 != slashPos) {
        fileName = fileName.mid(slashPos + 1);
    }
    if (fileName.isEmpty()) {
        fileName = QStringLiteral("unnamed");
    }
    fname += QLatin1Char('/') + fileName;

    qCDebug(MESSAGEVIEWER_LOG) << "Create temp file: " << fname;
    QByteArray data = node->decodedContent();
    if (node->contentType()->isText() && data.size() > 0) {
        // convert CRLF to LF before writing text attachments to disk
        data = KMime::CRLFtoLF(data);
    }
    QFile f(fname);
    if (!f.open(QIODevice::ReadWrite)) {
        qCWarning(MESSAGEVIEWER_LOG) << "Failed to write note to file:" << f.errorString();
        return QString();
    }
    f.write(data);
    // mAttachmentFilesDir->addTempFile(fname);
    // make file read-only so that nobody gets the impression that he might
    // edit attached files (cf. bug #52813)
    f.setPermissions(QFileDevice::ReadUser);
    f.close();

    return fname;
}

QUrl NodeHelper::tempFileUrlFromNode(const KMime::Content *node)
{
    if (!node) {
        return QUrl();
    }

    const QString index = persistentIndex(node);

    // foreach (const QString &path, mAttachmentFilesDir->temporaryFiles()) {
    //     const int right = path.lastIndexOf(QLatin1Char('/'));
    //     int left = path.lastIndexOf(QLatin1String(".index."), right);
    //     if (left != -1) {
    //         left += 7;
    //     }
    //
    //     QStringRef storedIndex(&path, left, right - left);
    //     if (left != -1 && storedIndex == index) {
    //         return QUrl::fromLocalFile(path);
    //     }
    // }
    return QUrl();
}

QString NodeHelper::createTempDir(const QString &param)
{
    QTemporaryFile *tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/messageviewer_XXXXXX") + QLatin1String(".index.") + param);
    tempFile->open();
    QString fname = tempFile->fileName();
    delete tempFile;

    if (::access(QFile::encodeName(fname), W_OK) != 0) {
        // Not there or not writable
        // if (KDE_mkdir(QFile::encodeName(fname), 0) != 0 ||
        //         ::chmod(QFile::encodeName(fname), S_IRWXU) != 0) {
        //     return QString(); //failed create
        // }
    }

    Q_ASSERT(!fname.isNull());

    // mAttachmentFilesDir->addTempDir(fname);
    return fname;
}

void NodeHelper::forceCleanTempFiles()
{
    // mAttachmentFilesDir->forceCleanTempFiles();
    // delete mAttachmentFilesDir;
    // mAttachmentFilesDir = 0;
}

void NodeHelper::removeTempFiles()
{
    //Don't delete it it will delete in class
    // mAttachmentFilesDir->removeTempFiles();
    // mAttachmentFilesDir = new PimCommon::AttachmentTemporaryFilesDirs();
}

void NodeHelper::addTempFile(const QString &file)
{
    // mAttachmentFilesDir->addTempFile(file);
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

    if (encryptionState(node) == KMMsgNotEncrypted) {
        // NOTE: children are tested ONLY when parent is not encrypted
        KMime::Content *child = MessageCore::NodeHelper::firstChild(node);
        if (child) {
            myState = overallEncryptionState(child);
        } else {
            myState = KMMsgNotEncrypted;
        }
    } else { // part is partially or fully encrypted
        myState = encryptionState(node);
    }
    // siblings are tested always
    KMime::Content *next = MessageCore::NodeHelper::nextSibling(node);
    if (next) {
        KMMsgEncryptionState otherState = overallEncryptionState(next);
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

    qCDebug(MESSAGEVIEWER_LOG) << "\n\n  KMMsgEncryptionState:" << myState;

    return myState;
}

KMMsgSignatureState NodeHelper::overallSignatureState(KMime::Content *node) const
{
    KMMsgSignatureState myState = KMMsgSignatureStateUnknown;
    if (!node) {
        return myState;
    }

    if (signatureState(node) == KMMsgNotSigned) {
        // children are tested ONLY when parent is not signed
        KMime::Content *child = MessageCore::NodeHelper::firstChild(node);
        if (child) {
            myState = overallSignatureState(child);
        } else {
            myState = KMMsgNotSigned;
        }
    } else { // part is partially or fully signed
        myState = signatureState(node);
    }
    // siblings are tested always
    KMime::Content *next = MessageCore::NodeHelper::nextSibling(node);
    if (next) {
        KMMsgSignatureState otherState = overallSignatureState(next);
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

    qCDebug(MESSAGEVIEWER_LOG) << "\n\n  KMMsgSignatureState:" << myState;

    return myState;
}

QString NodeHelper::iconName(KMime::Content *node, int size)
{
    if (!node) {
        return QString();
    }

    QByteArray mimeType = node->contentType()->mimeType();
    if (mimeType.isNull() || mimeType == "application/octet-stream") {
        const QString mime = Util::mimetype(node->contentDisposition()->filename()).name();
        mimeType = mime.toLatin1();
    }
    mimeType = mimeType.toLower();
    return Util::fileNameForMimetype(QLatin1String(mimeType), size, node->contentDisposition()->filename(),
                                     node->contentType()->name());
}

void NodeHelper::magicSetType(KMime::Content *node, bool aAutoDecode)
{
    const QByteArray body = (aAutoDecode) ? node->decodedContent() : node->body();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForData(body);

    QString mimetype = mime.name();
    node->contentType()->setMimeType(mimetype.toLatin1());
}

// static
QString NodeHelper::replacePrefixes(const QString &str,
                                    const QStringList &prefixRegExps,
                                    bool replace,
                                    const QString &newPrefix)
{
    bool recognized = false;
    // construct a big regexp that
    // 1. is anchored to the beginning of str (sans whitespace)
    // 2. matches at least one of the part regexps in prefixRegExps
    QString bigRegExp = QStringLiteral("^(?:\\s+|(?:%1))+\\s*")
                        .arg(prefixRegExps.join(QStringLiteral(")|(?:")));
    QRegExp rx(bigRegExp, Qt::CaseInsensitive);
    if (!rx.isValid()) {
        qCWarning(MESSAGEVIEWER_LOG) << "bigRegExp = \""
                                     << bigRegExp << "\"\n"
                                     << "prefix regexp is invalid!";
        // try good ole Re/Fwd:
        recognized = str.startsWith(newPrefix);
    } else { // valid rx
        QString tmp = str;
        if (rx.indexIn(tmp) == 0) {
            recognized = true;
            if (replace) {
                return tmp.replace(0, rx.matchedLength(), newPrefix + QLatin1Char(' '));
            }
        }
    }
    if (!recognized) {
        return newPrefix + QLatin1Char(' ') + str;
    } else {
        return str;
    }
}

QString NodeHelper::cleanSubject(KMime::Message *message)
{
    return cleanSubject(message, replySubjPrefixes + forwardSubjPrefixes,
                        true, QString()).trimmed();
}

QString NodeHelper::cleanSubject(KMime::Message *message,
                                 const QStringList &prefixRegExps,
                                 bool replace,
                                 const QString &newPrefix)
{
    QString cleanStr;
    if (message) {
        cleanStr =
            NodeHelper::replacePrefixes(
                message->subject()->asUnicodeString(), prefixRegExps, replace, newPrefix);
    }
    return cleanStr;
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

    const QTextCodec *c = mOverrideCodecs.value(node, 0);
    if (!c) {
        // no override-codec set for this message, try the CT charset parameter:
        c = codecForName(node->contentType()->charset());
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
        return 0;
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
        return 0;
    }
    const QMap<QByteArray, Interface::BodyPartMemento *>::const_iterator it =
        nit->find(which.toLower());
    return it != nit->end() ? it.value() : 0;
}

//FIXME(Andras) review it (by Marc?) to see if I got it right. This is supposed to be the partNode::internalSetBodyPartMemento replacement
void NodeHelper::setBodyPartMemento(KMime::Content *node, const QByteArray &which,
                                    Interface::BodyPartMemento *memento)
{
    QMap<QByteArray, Interface::BodyPartMemento *> &mementos
        = mBodyPartMementoMap[persistentIndex(node)];

    const QMap<QByteArray, Interface::BodyPartMemento *>::iterator it =
        mementos.lowerBound(which.toLower());

    if (it != mementos.end() && it.key() == which.toLower()) {
        delete it.value();
        if (memento) {
            it.value() = memento;
        } else {
            mementos.erase(it);
        }
    } else {
        mementos.insert(which.toLower(), memento);
    }
}

bool NodeHelper::isNodeDisplayedEmbedded(KMime::Content *node) const
{
    qCDebug(MESSAGEVIEWER_LOG) << "IS NODE: " << mDisplayEmbeddedNodes.contains(node);
    return mDisplayEmbeddedNodes.contains(node);
}

void NodeHelper::setNodeDisplayedEmbedded(KMime::Content *node, bool displayedEmbedded)
{
    qCDebug(MESSAGEVIEWER_LOG) << "SET NODE: " << node << displayedEmbedded;
    if (displayedEmbedded) {
        mDisplayEmbeddedNodes.insert(node);
    } else {
        mDisplayEmbeddedNodes.remove(node);
    }
}

bool NodeHelper::isNodeDisplayedHidden(KMime::Content *node) const
{
    return mDisplayHiddenNodes.contains(node);
}

void NodeHelper::setNodeDisplayedHidden(KMime::Content *node, bool displayedHidden)
{
    if (displayedHidden) {
        mDisplayHiddenNodes.insert(node);
    } else {
        mDisplayEmbeddedNodes.remove(node);
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
    const KMime::Content *const topLevel = node->topLevel();
    //if the node is an extra node, prepend the index of the extra node to the url
    Q_FOREACH (KMime::Content *realNode, mExtraContents.keys()) {
        const QList<KMime::Content *> &extraNodes = extraContents(realNode);
        const int extraNodesSize(extraNodes.size());
        for (int i = 0; i < extraNodesSize; ++i) {
            if (topLevel == extraNodes[i]) {
                indexStr = indexStr.prepend(QString::fromLatin1("%1:").arg(i));
                const QString outsideIndex =  persistentIndex(realNode);
                if (!outsideIndex.isEmpty()) {
                    indexStr = QString::fromLatin1("%1:").arg(outsideIndex) + indexStr;
                }
            }
        }
    }
    return indexStr;
}

KMime::Content *NodeHelper::contentFromIndex(KMime::Content *node, const QString &persistentIndex) const
{
    KMime::Content *topLevel = node->topLevel();
    if (persistentIndex.contains(QLatin1Char(':'))) {
        //if the content was not found, it might be in an extra node. Get the index of the extra node (the first part of the url),
        //and use the remaining part as a ContentIndex to find the node inside the extra node
        QString left = persistentIndex.left(persistentIndex.indexOf(QLatin1Char(':')));
        QString index = persistentIndex.mid(persistentIndex.indexOf(QLatin1Char(':')) + 1);

        QList<KMime::Content *> extras = extraContents(topLevel);

        if (index.contains(QLatin1Char(':'))) {
            extras = extraContents(topLevel->content(KMime::ContentIndex(left)));
            left = index.left(index.indexOf(QLatin1Char(':')));
            index = index.mid(index.indexOf(QLatin1Char(':')) + 1);
        }
        const KMime::ContentIndex idx(index);
        const int i = left.toInt();
        if (i >= 0 && i < extras.size()) {
            const KMime::Content *c = extras[i];
            return c->content(idx);
        }
    } else {
        if (topLevel) {
            return topLevel->content(KMime::ContentIndex(persistentIndex));
        }
    }
    return 0;
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
        const QRegExp rIndex(QStringLiteral("\\D\\.([0-9.:]+)/"));

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
    if (returnEncoding.toUpper().contains(QStringLiteral("ISO "))) {
        returnEncoding = returnEncoding.toUpper();
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

QString NodeHelper::fromAsString(KMime::Content *node)
{
    KMime::Message *topLevel = dynamic_cast<KMime::Message *>(node->topLevel());
    if (topLevel) {
        return topLevel->from()->asUnicodeString();
    }
    return QString();
}

void NodeHelper::attachExtraContent(KMime::Content *topLevelNode, KMime::Content *content)
{
    qCDebug(MESSAGEVIEWER_LOG) << "mExtraContents added for" << topLevelNode << " extra content: " << content;
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

    QList<KMime::Content * > extraNodes = extraContents(node);
    Q_FOREACH (KMime::Content *extra, extraNodes) {
        if (node->bodyIsMessage()) {
            qCWarning(MESSAGEVIEWER_LOG) << "Asked to attach extra content to a kmime::message, this does not make sense. Attaching to:" << node <<
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
    QList<KMime::Content * > extraNodes = extraContents(node);
    Q_FOREACH (KMime::Content *extra, extraNodes) {
        QByteArray s = extra->encodedContent();
        auto children = node->contents();
        Q_FOREACH (KMime::Content *c, children) {
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
        return 0;
    }

    mergeExtraNodes(topLevelNode);

    KMime::Message *m = new KMime::Message;
    m->setContent(topLevelNode->encodedContent());
    m->parse();

    cleanFromExtraNodes(topLevelNode);
//   qCDebug(MESSAGEVIEWER_LOG) << "MESSAGE WITH EXTRA: " << m->encodedContent();
//   qCDebug(MESSAGEVIEWER_LOG) << "MESSAGE WITHOUT EXTRA: " << topLevelNode->encodedContent();

    return m;
}

NodeHelper::AttachmentDisplayInfo NodeHelper::attachmentDisplayInfo(KMime::Content *node)
{
    AttachmentDisplayInfo info;
    // info.icon = iconName(node, KIconLoader::Small);
    const QString name = node->contentType()->name();
    info.label = name.isEmpty() ? fileName(node) : name;
    if (info.label.isEmpty()) {
        info.label = node->contentDescription()->asUnicodeString();
    }

    bool typeBlacklisted = node->contentType()->mediaType().toLower() == "multipart";
    if (!typeBlacklisted) {
        typeBlacklisted = KMime::isCryptoPart(node);
    }
    typeBlacklisted = typeBlacklisted || node == node->topLevel();
    const bool firstTextChildOfEncapsulatedMsg =
        node->contentType()->mediaType().toLower() == "text" &&
        node->contentType()->subType().toLower() == "plain" &&
        node->parent() && node->parent()->contentType()->mediaType().toLower() == "message";
    typeBlacklisted = typeBlacklisted || firstTextChildOfEncapsulatedMsg;
    info.displayInHeader = !info.label.isEmpty() && !info.icon.isEmpty() && !typeBlacklisted;
    return info;
}

KMime::Content *NodeHelper::decryptedNodeForContent(KMime::Content *content) const
{
    const QList<KMime::Content *> xc = extraContents(content);
    if (!xc.empty()) {
        if (xc.size() == 1) {
            return xc.front();
        } else {
            qCWarning(MESSAGEVIEWER_LOG) << "WTF, encrypted node has multiple extra contents?";
        }
    }
    return 0;
}

bool NodeHelper::unencryptedMessage_helper(KMime::Content *node, QByteArray &resultingData, bool addHeaders,
        int recursionLevel)
{
    bool returnValue = false;
    if (node) {
        KMime::Content *curNode = node;
        KMime::Content *decryptedNode = 0;
        const QByteArray type = node->contentType(false) ? QByteArray(node->contentType()->mediaType()).toLower() : "text";
        const QByteArray subType = node->contentType(false) ? node->contentType()->subType().toLower() : "plain";
        const bool isMultipart = node->contentType(false) && node->contentType()->isMultipart();
        bool isSignature = false;

        qCDebug(MESSAGEVIEWER_LOG) << "(" << recursionLevel << ") Looking at" << type << "/" << subType;

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
            qCDebug(MESSAGEVIEWER_LOG) << "Current node has an associated decrypted node, adding a modified header "
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
            qCDebug(MESSAGEVIEWER_LOG) << "Current node is a signature, adding it as-is.";
            // We can't change the nodes under the signature, as that would invalidate it. Add the signature
            // and its child as-is
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }
            resultingData += curNode->encodedBody();
            returnValue = false;
        }

        else if (isMultipart) {
            qCDebug(MESSAGEVIEWER_LOG) << "Current node is a multipart node, adding its header and then processing all children.";
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
            qCDebug(MESSAGEVIEWER_LOG) << "Current node is a message, adding the header and then processing the child.";
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }

            returnValue = unencryptedMessage_helper(curNode->bodyAsMessage().data(), resultingData, true, recursionLevel + 1);
        }

        else {
            qCDebug(MESSAGEVIEWER_LOG) << "Current node is an ordinary leaf node, adding it as-is.";
            if (addHeaders) {
                resultingData += curNode->head() + '\n';
            }
            resultingData += curNode->body();
            returnValue = false;
        }
    }

    qCDebug(MESSAGEVIEWER_LOG) << "(" << recursionLevel << ") done.";
    return returnValue;
}

KMime::Message::Ptr NodeHelper::unencryptedMessage(const KMime::Message::Ptr &originalMessage)
{
    QByteArray resultingData;
    const bool messageChanged = unencryptedMessage_helper(originalMessage.data(), resultingData, true);
    if (messageChanged) {
#if 0
        qCDebug(MESSAGEVIEWER_LOG) << "Resulting data is:" << resultingData;
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
