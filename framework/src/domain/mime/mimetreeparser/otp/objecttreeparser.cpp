/*
    objecttreeparser.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>
    Copyright (C) 2002-2004 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>
    Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

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

// MessageViewer includes

#include "objecttreeparser.h"

#include "attachmentstrategy.h"
#include "bodypartformatterbasefactory.h"
#include "nodehelper.h"
#include "messagepart.h"
#include "partnodebodypart.h"

#include "mimetreeparser_debug.h"

#include "utils.h"
#include "bodypartformatter.h"
#include "util.h"

#include <KMime/Headers>
#include <KMime/Message>

// KDE includes

// Qt includes
#include <QByteArray>
#include <QTextCodec>
#include <QUrl>

using namespace MimeTreeParser;

ObjectTreeParser::ObjectTreeParser(const ObjectTreeParser *topLevelParser,
                                   bool showOnlyOneMimePart,
                                   const AttachmentStrategy *strategy)
    : mSource(topLevelParser->mSource),
      mNodeHelper(topLevelParser->mNodeHelper),
      mTopLevelContent(topLevelParser->mTopLevelContent),
      mShowOnlyOneMimePart(showOnlyOneMimePart),
      mHasPendingAsyncJobs(false),
      mAllowAsync(topLevelParser->mAllowAsync),
      mAttachmentStrategy(strategy)
{
    init();
}

ObjectTreeParser::ObjectTreeParser(Interface::ObjectTreeSource *source,
                                   MimeTreeParser::NodeHelper *nodeHelper,
                                   bool showOnlyOneMimePart,
                                   const AttachmentStrategy *strategy)
    : mSource(source),
      mNodeHelper(nodeHelper),
      mTopLevelContent(nullptr),
      mShowOnlyOneMimePart(showOnlyOneMimePart),
      mHasPendingAsyncJobs(false),
      mAllowAsync(false),
      mAttachmentStrategy(strategy)
{
    init();
}

void ObjectTreeParser::init()
{
    Q_ASSERT(mSource);
    if (!attachmentStrategy()) {
        mAttachmentStrategy = mSource->attachmentStrategy();
    }

    if (!mNodeHelper) {
        mNodeHelper = new NodeHelper();
        mDeleteNodeHelper = true;
    } else {
        mDeleteNodeHelper = false;
    }
}

ObjectTreeParser::ObjectTreeParser(const ObjectTreeParser &other)
    : mSource(other.mSource),
      mNodeHelper(other.nodeHelper()),   //TODO(Andras) hm, review what happens if mDeleteNodeHelper was true in the source
      mTopLevelContent(other.mTopLevelContent),
      mShowOnlyOneMimePart(other.showOnlyOneMimePart()),
      mHasPendingAsyncJobs(other.hasPendingAsyncJobs()),
      mAllowAsync(other.allowAsync()),
      mAttachmentStrategy(other.attachmentStrategy()),
      mDeleteNodeHelper(false)
{

}

ObjectTreeParser::~ObjectTreeParser()
{
    if (mDeleteNodeHelper) {
        delete mNodeHelper;
        mNodeHelper = nullptr;
    }
}

void ObjectTreeParser::setAllowAsync(bool allow)
{
    Q_ASSERT(!mHasPendingAsyncJobs);
    mAllowAsync = allow;
}

bool ObjectTreeParser::allowAsync() const
{
    return mAllowAsync;
}

bool ObjectTreeParser::hasPendingAsyncJobs() const
{
    return mHasPendingAsyncJobs;
}

QString ObjectTreeParser::plainTextContent() const
{
    return mPlainTextContent;
}

QString ObjectTreeParser::htmlContent() const
{
    return mHtmlContent;
}

void ObjectTreeParser::copyContentFrom(const ObjectTreeParser *other)
{
    mPlainTextContent += other->plainTextContent();
    mHtmlContent += other->htmlContent();
    if (!other->plainTextContentCharset().isEmpty()) {
        mPlainTextContentCharset = other->plainTextContentCharset();
    }
    if (!other->htmlContentCharset().isEmpty()) {
        mHtmlContentCharset = other->htmlContentCharset();
    }
}

static KMime::Content *find(KMime::Content *node, const std::function<bool(KMime::Content *)> &select)
{
    QByteArray mediaType("text");
    QByteArray subType("plain");
    if (node->contentType(false) && !node->contentType()->mediaType().isEmpty() &&
            !node->contentType()->subType().isEmpty()) {
        mediaType = node->contentType()->mediaType();
        subType = node->contentType()->subType();
    }
    if (select(node)) {
        return node;
    }
    for (const auto c: node->contents()) {
        if (const auto n = find(c, select)) {
            return n;
        }
    }
    return nullptr;
}


KMime::Content *ObjectTreeParser::find(const std::function<bool(KMime::Content *)> &select)
{
    return ::find(mTopLevelContent, select);
}

static QVector<Interface::MessagePart::Ptr> collect(Interface::MessagePart::Ptr start, const std::function<bool(const MessagePartPtr &)> &filter, const std::function<bool(const MessagePartPtr &)> &select)
{
    MessagePartPtr ptr = start.dynamicCast<MessagePart>();
    Q_ASSERT(ptr);
    if (!filter(ptr)) {
        return {};
    }

    QVector<Interface::MessagePart::Ptr> list;
    if (select(ptr)) {
        list << start;
    }
    if (ptr) {
        for (const auto &p: ptr->subParts()) {
            list << ::collect(p, filter, select);
        }
    }
    return list;
}

QVector<Interface::MessagePart::Ptr> ObjectTreeParser::collectContentParts()
{
    QVector<Interface::MessagePart::Ptr> contentParts = ::collect(mParsedPart,
        [] (const MessagePartPtr &part) {
            // return p->type() != "EncapsulatedPart";
            return true;
        },
        [] (const MessagePartPtr &part) {
            if (const auto attachment = dynamic_cast<MimeTreeParser::AttachmentMessagePart*>(part.data())) {
            } else if (const auto text = dynamic_cast<MimeTreeParser::TextMessagePart*>(part.data())) {
                return true;
            } else if (const auto alternative = dynamic_cast<MimeTreeParser::AlternativeMessagePart*>(part.data())) {
                return true;
            } else if (const auto html = dynamic_cast<MimeTreeParser::HtmlMessagePart*>(part.data())) {
                return true;
            }
            return false;
        });
    return contentParts;
}

//-----------------------------------------------------------------------------

void ObjectTreeParser::parseObjectTree(const QByteArray &mimeMessage)
{
    const auto mailData = KMime::CRLFtoLF(mimeMessage);
    mMsg = KMime::Message::Ptr(new KMime::Message);
    mMsg->setContent(mailData);
    mMsg->parse();
    parseObjectTree(mMsg.data());
}

void ObjectTreeParser::parseObjectTree(KMime::Content *node)
{
    mTopLevelContent = node;
    mParsedPart = parseObjectTreeInternal(node, showOnlyOneMimePart());

    if (mParsedPart) {
        if (auto mp = toplevelTextNode(mParsedPart)) {
            if (auto _mp = mp.dynamicCast<TextMessagePart>()) {
                extractNodeInfos(_mp->mNode, true);
            } else if (auto _mp = mp.dynamicCast<AlternativeMessagePart>()) {
                if (_mp->mChildNodes.contains(Util::MultipartPlain)) {
                    extractNodeInfos(_mp->mChildNodes[Util::MultipartPlain], true);
                }
            }
            setPlainTextContent(mp->text());
        }

    }
}

MessagePartPtr ObjectTreeParser::parsedPart() const
{
    return mParsedPart;
}

bool ObjectTreeParser::processType(KMime::Content *node, ProcessResult &processResult, const QByteArray &mediaType, const QByteArray &subType, Interface::MessagePartPtr &mpRet, bool onlyOneMimePart)
{
    bool bRendered = false;
    const auto sub = mSource->bodyPartFormatterFactory()->subtypeRegistry(mediaType.constData());
    auto range =  sub.equal_range(subType.constData());
    for (auto it = range.first; it != range.second; ++it) {
        const auto formatter = (*it).second;
        if (!formatter) {
            continue;
        }
        PartNodeBodyPart part(this, &processResult, mTopLevelContent, node, mNodeHelper);
        // Set the default display strategy for this body part relying on the
        // identity of Interface::BodyPart::Display and AttachmentStrategy::Display
        part.setDefaultDisplay((Interface::BodyPart::Display) attachmentStrategy()->defaultDisplay(node));


        const Interface::MessagePart::Ptr result = formatter->process(part);
        if (!result) {
            continue;
        }

        if (const auto mp = result.dynamicCast<MessagePart>()) {
            mp->setAttachmentFlag(node);
            mpRet = result;
            bRendered = true;
            break;
        } else {
            continue;
        }
    }
    return bRendered;
}

MessagePart::Ptr ObjectTreeParser::parseObjectTreeInternal(KMime::Content *node, bool onlyOneMimePart)
{
    if (!node) {
        return MessagePart::Ptr();
    }

    // reset pending async jobs state (we'll rediscover pending jobs as we go)
    mHasPendingAsyncJobs = false;

    // reset "processed" flags for...
    if (onlyOneMimePart) {
        // ... this node and all descendants
        mNodeHelper->setNodeUnprocessed(node, false);
        if (!node->contents().isEmpty()) {
            mNodeHelper->setNodeUnprocessed(node, true);
        }
    } else if (!node->parent()) {
        // ...this node and all it's siblings and descendants
        mNodeHelper->setNodeUnprocessed(node, true);
    }

    const bool isRoot = node->isTopLevel();
    auto parsedPart = MessagePart::Ptr(new MessagePartList(this));
    parsedPart->setIsRoot(isRoot);
    KMime::Content *parent = node->parent();
    auto contents = parent ? parent->contents() : KMime::Content::List();
    if (contents.isEmpty()) {
        contents.append(node);
    }
    int i = contents.indexOf(const_cast<KMime::Content *>(node));
    for (; i < contents.size(); ++i) {
        node = contents.at(i);
        if (mNodeHelper->nodeProcessed(node)) {
            continue;
        }

        ProcessResult processResult(mNodeHelper);

        QByteArray mediaType("text");
        QByteArray subType("plain");
        if (node->contentType(false) && !node->contentType()->mediaType().isEmpty() &&
                !node->contentType()->subType().isEmpty()) {
            mediaType = node->contentType()->mediaType();
            subType = node->contentType()->subType();
        }

        Interface::MessagePartPtr mp;
        if (processType(node, processResult, mediaType, subType, mp, onlyOneMimePart)) {
            if (mp) {
                parsedPart->appendSubPart(mp);
            }
        } else if (processType(node, processResult, mediaType, "*", mp, onlyOneMimePart)) {
            if (mp) {
                parsedPart->appendSubPart(mp);
            }
        } else {
            qCWarning(MIMETREEPARSER_LOG) << "THIS SHOULD NO LONGER HAPPEN:" << mediaType << '/' << subType;
            const auto mp = defaultHandling(node, processResult, onlyOneMimePart);
            if (mp) {
                if (auto _mp = mp.dynamicCast<MessagePart>()) {
                    _mp->setAttachmentFlag(node);
                }
                parsedPart->appendSubPart(mp);
            }
        }
        mNodeHelper->setNodeProcessed(node, false);

        // adjust signed/encrypted flags if inline PGP was found
        processResult.adjustCryptoStatesOfNode(node);

        if (onlyOneMimePart) {
            break;
        }
    }

    return parsedPart;
}

Interface::MessagePart::Ptr ObjectTreeParser::defaultHandling(KMime::Content *node, ProcessResult &result, bool onlyOneMimePart)
{
    Interface::MessagePart::Ptr mp;
    ProcessResult processResult(mNodeHelper);

    if (node->contentType()->mimeType() == QByteArrayLiteral("application/octet-stream") &&
            (node->contentType()->name().endsWith(QLatin1String("p7m")) ||
             node->contentType()->name().endsWith(QLatin1String("p7s")) ||
             node->contentType()->name().endsWith(QLatin1String("p7c"))
            ) &&
            processType(node, processResult, "application", "pkcs7-mime", mp, onlyOneMimePart)) {
        return mp;
    }

    const auto _mp = AttachmentMessagePart::Ptr(new AttachmentMessagePart(this, node, mSource->decryptMessage()));
    result.setInlineSignatureState(_mp->signatureState());
    result.setInlineEncryptionState(_mp->encryptionState());
    mp = _mp;

    // always show images in multipart/related when showing in html, not with an additional icon

    // Show it inline if showOnlyOneMimePart(), which means the user clicked the image
    // in the message structure viewer manually, and therefore wants to see the full image

    return mp;
}

KMMsgSignatureState ProcessResult::inlineSignatureState() const
{
    return mInlineSignatureState;
}

void ProcessResult::setInlineSignatureState(KMMsgSignatureState state)
{
    mInlineSignatureState = state;
}

KMMsgEncryptionState ProcessResult::inlineEncryptionState() const
{
    return mInlineEncryptionState;
}

void ProcessResult::setInlineEncryptionState(KMMsgEncryptionState state)
{
    mInlineEncryptionState = state;
}

void ProcessResult::adjustCryptoStatesOfNode(const KMime::Content *node) const
{
    if ((inlineSignatureState()  != KMMsgNotSigned) ||
            (inlineEncryptionState() != KMMsgNotEncrypted)) {
        mNodeHelper->setSignatureState(node, inlineSignatureState());
        mNodeHelper->setEncryptionState(node, inlineEncryptionState());
    }
}

void ObjectTreeParser::extractNodeInfos(KMime::Content *curNode, bool isFirstTextPart)
{
    if (isFirstTextPart) {
        mPlainTextContent += curNode->decodedText();
        mPlainTextContentCharset += NodeHelper::charset(curNode);
    }
}

void ObjectTreeParser::setPlainTextContent(const QString &plainTextContent)
{
    mPlainTextContent = plainTextContent;
}

const QTextCodec *ObjectTreeParser::codecFor(KMime::Content *node) const
{
    Q_ASSERT(node);
    if (mSource->overrideCodec()) {
        return mSource->overrideCodec();
    }
    return mNodeHelper->codec(node);
}

QByteArray ObjectTreeParser::plainTextContentCharset() const
{
    return mPlainTextContentCharset;
}

QByteArray ObjectTreeParser::htmlContentCharset() const
{
    return mHtmlContentCharset;
}

bool ObjectTreeParser::showOnlyOneMimePart() const
{
    return mShowOnlyOneMimePart;
}

void ObjectTreeParser::setShowOnlyOneMimePart(bool show)
{
    mShowOnlyOneMimePart = show;
}

const AttachmentStrategy *ObjectTreeParser::attachmentStrategy() const
{
    return mAttachmentStrategy;
}


MimeTreeParser::NodeHelper *ObjectTreeParser::nodeHelper() const
{
    return mNodeHelper;
}
