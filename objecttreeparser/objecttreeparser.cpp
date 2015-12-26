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

#include "memento/verifydetachedbodypartmemento.h"
#include "memento/verifyopaquebodypartmemento.h"
#include "memento/cryptobodypartmemento.h"
#include "memento/decryptverifybodypartmemento.h"
#include "messagepart.h"
#include "objecttreesourceif.h"

#include "viewer/viewer_p.h"
#include "partmetadata.h"
#include "attachmentstrategy.h"
#include "interfaces/htmlwriter.h"
#include "widgets/htmlstatusbar.h"
#include "csshelper.h"
#include "viewer/bodypartformatterfactory.h"
#include "viewer/partnodebodypart.h"
#include "interfaces/bodypartformatter.h"
#include "settings/messageviewersettings.h"
#include "messageviewer/messageviewerutil.h"
#include "job/kleojobexecutor.h"
#include "messageviewer/nodehelper.h"
#include "utils/iconnamecache.h"
#include "viewer/htmlquotecolorer.h"
#include "messageviewer_debug.h"
#include "converthtmltoplaintext.h"

// KDEPIM includes
#include <MessageCore/StringUtil>
#include <Libkleo/SpecialJob>
#include <Libkleo/CryptoBackendFactory>
#include <Libkleo/DecryptVerifyJob>
#include <Libkleo/VerifyDetachedJob>
#include <Libkleo/VerifyOpaqueJob>
#include <Libkleo/KeyListJob>
#include <Libkleo/ImportJob>
#include <Libkleo/Dn>
#include "cryptohelper.h"

// KDEPIMLIBS includes
#include <gpgme++/importresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>
#include <gpgme.h>
#include <kmime/kmime_message.h>
#include <kmime/kmime_headers.h>

// KDE includes

#include <KLocalizedString>
#include <QTemporaryFile>

#include <kcodecs.h>
#include <kconfiggroup.h>

#include <KEmailAddress>
#include <KTextToHTML>

// Qt includes
#include <QApplication>
#include <QTextDocument>
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QByteArray>
#include <QBuffer>
#include <QPixmap>
#include <QPainter>
#include <QPointer>
#include <QWebPage>
#include <QWebFrame>
#include <QUrlQuery>

// other includes
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <MessageCore/NodeHelper>
#include <qtextdocument.h>

using namespace MessageViewer;
using namespace MessageCore;

// A small class that eases temporary CryptPlugWrapper changes:
class ObjectTreeParser::CryptoProtocolSaver
{
    ObjectTreeParser *otp;
    const Kleo::CryptoBackend::Protocol *protocol;
public:
    CryptoProtocolSaver(ObjectTreeParser *_otp, const Kleo::CryptoBackend::Protocol *_w)
        : otp(_otp), protocol(_otp ? _otp->cryptoProtocol() : 0)
    {
        if (otp) {
            otp->setCryptoProtocol(_w);
        }
    }

    ~CryptoProtocolSaver()
    {
        if (otp) {
            otp->setCryptoProtocol(protocol);
        }
    }
};

ObjectTreeParser::ObjectTreeParser(const ObjectTreeParser *topLevelParser,
                                   bool showOnlyOneMimePart,
                                   const AttachmentStrategy *strategy)
    : mSource(topLevelParser->mSource),
      mNodeHelper(topLevelParser->mNodeHelper),
      mHtmlWriter(topLevelParser->mHtmlWriter),
      mTopLevelContent(topLevelParser->mTopLevelContent),
      mCryptoProtocol(topLevelParser->mCryptoProtocol),
      mShowOnlyOneMimePart(showOnlyOneMimePart),
      mHasPendingAsyncJobs(false),
      mAllowAsync(topLevelParser->mAllowAsync),
      mAttachmentStrategy(strategy),
      mPrinting(false)
{
    init();
}

ObjectTreeParser::ObjectTreeParser(ObjectTreeSourceIf *source,
                                   MessageViewer::NodeHelper *nodeHelper,
                                   const Kleo::CryptoBackend::Protocol *protocol,
                                   bool showOnlyOneMimePart,
                                   const AttachmentStrategy *strategy)
    : mSource(source),
      mNodeHelper(nodeHelper),
      mHtmlWriter(0),
      mTopLevelContent(0),
      mCryptoProtocol(protocol),
      mShowOnlyOneMimePart(showOnlyOneMimePart),
      mHasPendingAsyncJobs(false),
      mAllowAsync(false),
      mAttachmentStrategy(strategy),
      mPrinting(false)
{
    init();
}

void ObjectTreeParser::init()
{
    assert(mSource);
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
      mHtmlWriter(other.mHtmlWriter),
      mTopLevelContent(other.mTopLevelContent),
      mCryptoProtocol(other.cryptoProtocol()),
      mShowOnlyOneMimePart(other.showOnlyOneMimePart()),
      mHasPendingAsyncJobs(other.hasPendingAsyncJobs()),
      mAllowAsync(other.allowAsync()),
      mAttachmentStrategy(other.attachmentStrategy()),
      mDeleteNodeHelper(false)   // TODO see above
{

}

ObjectTreeParser::~ObjectTreeParser()
{
    if (mDeleteNodeHelper) {
        delete mNodeHelper;
        mNodeHelper = 0;
    }
}

void ObjectTreeParser::setAllowAsync(bool allow)
{
    assert(!mHasPendingAsyncJobs);
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

void ObjectTreeParser::createAndParseTempNode(KMime::Content *parentNode, const char *content, const char *cntDesc)
{
    //  qCDebug(MESSAGEVIEWER_LOG) << "CONTENT: " << QByteArray( content ).left( 100 ) << " CNTDESC: " << cntDesc;

    KMime::Content *newNode = new KMime::Content();
    newNode->setContent(KMime::CRLFtoLF(content));
    newNode->parse();
    /*
    qCDebug(MESSAGEVIEWER_LOG)  << "MEDIATYPE: " << newNode->contentType()->mediaType() << newNode->contentType()->mimeType()    ;
    qCDebug(MESSAGEVIEWER_LOG) << "DECODEDCONTENT: " << newNode->decodedContent().left(400);
    qCDebug(MESSAGEVIEWER_LOG) << "ENCODEDCONTENT: " << newNode->encodedContent().left(400);
    qCDebug(MESSAGEVIEWER_LOG) << "BODY: " << newNode->body().left(400);
    */

    if (!newNode->head().isEmpty()) {
        newNode->contentDescription()->from7BitString(cntDesc);
    }
    mNodeHelper->attachExtraContent(parentNode, newNode);

    ObjectTreeParser otp(this);
    otp.parseObjectTreeInternal(newNode);
    copyContentFrom(&otp);
}

//-----------------------------------------------------------------------------

void ObjectTreeParser::parseObjectTree(KMime::Content *node)
{
    mTopLevelContent = node;
    parseObjectTreeInternal(node);
}

void ObjectTreeParser::setPrinting(bool printing)
{
    mPrinting = printing;
}

void ObjectTreeParser::parseObjectTreeInternal(KMime::Content *node)
{
    if (!node) {
        return;
    }

    // reset pending async jobs state (we'll rediscover pending jobs as we go)
    mHasPendingAsyncJobs = false;

    // reset "processed" flags for...
    if (showOnlyOneMimePart()) {
        // ... this node and all descendants
        mNodeHelper->setNodeUnprocessed(node, false);
        if (MessageCore::NodeHelper::firstChild(node)) {
            mNodeHelper->setNodeUnprocessed(node, true);
        }
    } else if (!node->parent()) {
        // ...this node and all it's siblings and descendants
        mNodeHelper->setNodeUnprocessed(node, true);
    }

    // Make sure the whole content is relative, so that nothing is painted over the header
    // if a malicious message uses absolute positioning.
    // Also force word wrapping, which is useful for printing, see https://issues.kolab.org/issue3992.
    bool isRoot = node->isTopLevel();
    if (isRoot && htmlWriter()) {
        htmlWriter()->queue(QStringLiteral("<div style=\"position: relative; word-wrap: break-word\">\n"));
    }

    for (; node; node = MessageCore::NodeHelper::nextSibling(node)) {
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

        // First, try if an external plugin can handle this MIME part
        if (const Interface::BodyPartFormatter * formatter
                = BodyPartFormatterFactory::instance()->createFor(mediaType, subType)) {
            PartNodeBodyPart part(this, &processResult, mTopLevelContent, node, mNodeHelper, codecFor(node));
            // Set the default display strategy for this body part relying on the
            // identity of Interface::BodyPart::Display and AttachmentStrategy::Display
            part.setDefaultDisplay((Interface::BodyPart::Display) attachmentStrategy()->defaultDisplay(node));

            mNodeHelper->setNodeDisplayedEmbedded(node, true);

            AttachmentMarkBlock block(htmlWriter(), node);
            QObject *asyncResultObserver = allowAsync() ? mSource->sourceObject() : 0;
            const Interface::BodyPartFormatter::Result result = formatter->format(&part, htmlWriter(), asyncResultObserver);
            switch (result) {
            case Interface::BodyPartFormatter::AsIcon:
                processResult.setNeverDisplayInline(true);
                mNodeHelper->setNodeDisplayedEmbedded(node, false);
            // fall through:
            case Interface::BodyPartFormatter::Failed:
            {
                const auto mp = defaultHandling(node, processResult);
                if (mp) {
                    mp->html(false);
                }
                break;
            }
            case Interface::BodyPartFormatter::Ok:
            case Interface::BodyPartFormatter::NeedContent:
                // FIXME: incomplete content handling
                ;
            }

            // No external plugin can handle the MIME part, handle it internally
        } else {
            qCCritical(MESSAGEVIEWER_LOG) << "THIS SHOULD NO LONGER HAPPEN:" << mediaType << '/' << subType;
            AttachmentMarkBlock block(htmlWriter(), node);
            const auto mp = defaultHandling(node, processResult);
            if (mp) {
                mp->html(false);
            }
        }
        mNodeHelper->setNodeProcessed(node, false);

        // adjust signed/encrypted flags if inline PGP was found
        processResult.adjustCryptoStatesOfNode(node);

        if (showOnlyOneMimePart()) {
            break;
        }
    }

    if (isRoot && htmlWriter()) {
        htmlWriter()->queue(QStringLiteral("</div>\n"));
    }
}

MessagePart::Ptr ObjectTreeParser::defaultHandling(KMime::Content *node, ProcessResult &result)
{
    // ### (mmutz) default handling should go into the respective
    // ### bodypartformatters.
    if (!htmlWriter()) {
        qCWarning(MESSAGEVIEWER_LOG) << "no htmlWriter()";
        return MessagePart::Ptr();
    }

    // always show images in multipart/related when showing in html, not with an additional icon
    if (result.isImage() && node->parent() &&
            node->parent()->contentType()->subType() == "related" && mSource->htmlMail() && !showOnlyOneMimePart()) {
        QString fileName = mNodeHelper->writeNodeToTempFile(node);
        QString href = QLatin1String("file:///") + fileName;
        QByteArray cid = node->contentID()->identifier();
        htmlWriter()->embedPart(cid, href);
        nodeHelper()->setNodeDisplayedEmbedded(node, true);
        return MessagePart::Ptr();
    }
    MessagePart::Ptr mp;
    if (node->contentType()->mimeType() == QByteArray("application/octet-stream") &&
            (node->contentType()->name().endsWith(QLatin1String("p7m")) ||
             node->contentType()->name().endsWith(QLatin1String("p7s")) ||
             node->contentType()->name().endsWith(QLatin1String("p7c"))
            ) &&
            (mp = processApplicationPkcs7MimeSubtype(node, result))) {
        return mp;
    }

    const AttachmentStrategy *const as = attachmentStrategy();
    if (as && as->defaultDisplay(node) == AttachmentStrategy::None &&
            !showOnlyOneMimePart() &&
            node->parent() /* message is not an attachment */) {
        mNodeHelper->setNodeDisplayedHidden(node, true);
        return MessagePart::Ptr();
    }

    bool asIcon = true;
    if (!result.neverDisplayInline())
        if (as) {
            asIcon = as->defaultDisplay(node) == AttachmentStrategy::AsIcon;
        }

    // Show it inline if showOnlyOneMimePart(), which means the user clicked the image
    // in the message structure viewer manually, and therefore wants to see the full image
    if (result.isImage() && showOnlyOneMimePart() && !result.neverDisplayInline()) {
        asIcon = false;
    }

    // neither image nor text -> show as icon
    if (!result.isImage()
            && !node->contentType()->isText()) {
        asIcon = true;
    }

    /*FIXME(Andras) port it
    // if the image is not complete do not try to show it inline
    if ( result.isImage() && !node->msgPart().isComplete() )
    asIcon = true;
    */

    if (asIcon) {
        if (!(as && as->defaultDisplay(node) == AttachmentStrategy::None) ||
                showOnlyOneMimePart()) {
            // Write the node as icon only
            writePartIcon(node);
        } else {
            mNodeHelper->setNodeDisplayedHidden(node, true);
        }
    } else if (result.isImage()) {
        // Embed the image
        mNodeHelper->setNodeDisplayedEmbedded(node, true);
        writePartIcon(node, true);
    } else {
        mNodeHelper->setNodeDisplayedEmbedded(node, true);
        const auto mp = TextMessagePart::Ptr(new TextMessagePart(this, node, false, false, mSource->decryptMessage()));
        result.setInlineSignatureState(mp->signatureState());
        result.setInlineEncryptionState(mp->encryptionState());
        return mp;
    }
    return MessagePart::Ptr();
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

bool ProcessResult::neverDisplayInline() const
{
    return mNeverDisplayInline;
}

void ProcessResult::setNeverDisplayInline(bool display)
{
    mNeverDisplayInline = display;
}

bool ProcessResult::isImage() const
{
    return mIsImage;
}

void ProcessResult::setIsImage(bool image)
{
    mIsImage = image;
}

void ProcessResult::adjustCryptoStatesOfNode(KMime::Content *node) const
{
    if ((inlineSignatureState()  != KMMsgNotSigned) ||
            (inlineEncryptionState() != KMMsgNotEncrypted)) {
        mNodeHelper->setSignatureState(node, inlineSignatureState());
        mNodeHelper->setEncryptionState(node, inlineEncryptionState());
    }
}

//////////////////
//////////////////
//////////////////

static int signatureToStatus(const GpgME::Signature &sig)
{
    switch (sig.status().code()) {
    case GPG_ERR_NO_ERROR:
        return GPGME_SIG_STAT_GOOD;
    case GPG_ERR_BAD_SIGNATURE:
        return GPGME_SIG_STAT_BAD;
    case GPG_ERR_NO_PUBKEY:
        return GPGME_SIG_STAT_NOKEY;
    case GPG_ERR_NO_DATA:
        return GPGME_SIG_STAT_NOSIG;
    case GPG_ERR_SIG_EXPIRED:
        return GPGME_SIG_STAT_GOOD_EXP;
    case GPG_ERR_KEY_EXPIRED:
        return GPGME_SIG_STAT_GOOD_EXPKEY;
    default:
        return GPGME_SIG_STAT_ERROR;
    }
}

void ObjectTreeParser::writeCertificateImportResult(const GpgME::ImportResult &res)
{
    if (res.error()) {
        htmlWriter()->queue(i18n("Sorry, certificate could not be imported.<br />"
                                 "Reason: %1", QString::fromLocal8Bit(res.error().asString())));
        return;
    }

    const int nImp = res.numImported();
    const int nUnc = res.numUnchanged();
    const int nSKImp = res.numSecretKeysImported();
    const int nSKUnc = res.numSecretKeysUnchanged();
    if (!nImp && !nSKImp && !nUnc && !nSKUnc) {
        htmlWriter()->queue(i18n("Sorry, no certificates were found in this message."));
        return;
    }
    QString comment = QLatin1String("<b>") + i18n("Certificate import status:") + QLatin1String("</b><br/>&nbsp;<br/>");
    if (nImp)
        comment += i18np("1 new certificate was imported.",
                         "%1 new certificates were imported.", nImp) + QLatin1String("<br/>");
    if (nUnc)
        comment += i18np("1 certificate was unchanged.",
                         "%1 certificates were unchanged.", nUnc) + QLatin1String("<br/>");
    if (nSKImp)
        comment += i18np("1 new secret key was imported.",
                         "%1 new secret keys were imported.", nSKImp) + QLatin1String("<br/>");
    if (nSKUnc)
        comment += i18np("1 secret key was unchanged.",
                         "%1 secret keys were unchanged.", nSKUnc) + QLatin1String("<br/>");
    comment += QLatin1String("&nbsp;<br/>");
    htmlWriter()->queue(comment);
    if (!nImp && !nSKImp) {
        htmlWriter()->queue(QStringLiteral("<hr>"));
        return;
    }
    const std::vector<GpgME::Import> imports = res.imports();
    if (imports.empty()) {
        htmlWriter()->queue(i18n("Sorry, no details on certificate import available.") + QLatin1String("<hr>"));
        return;
    }
    htmlWriter()->queue(QLatin1String("<b>") + i18n("Certificate import details:") + QLatin1String("</b><br/>"));
    std::vector<GpgME::Import>::const_iterator end(imports.end());
    for (std::vector<GpgME::Import>::const_iterator it = imports.begin(); it != end; ++it) {
        if ((*it).error()) {
            htmlWriter()->queue(i18nc("Certificate import failed.", "Failed: %1 (%2)", QLatin1String((*it).fingerprint()),
                                      QString::fromLocal8Bit((*it).error().asString())));
        } else if ((*it).status() & ~GpgME::Import::ContainedSecretKey) {
            if ((*it).status() & GpgME::Import::ContainedSecretKey) {
                htmlWriter()->queue(i18n("New or changed: %1 (secret key available)", QLatin1String((*it).fingerprint())));
            } else {
                htmlWriter()->queue(i18n("New or changed: %1", QLatin1String((*it).fingerprint())));
            }
        }
        htmlWriter()->queue(QStringLiteral("<br/>"));
    }

    htmlWriter()->queue(QStringLiteral("<hr>"));
}

bool ObjectTreeParser::okDecryptMIME(KMime::Content &data,
                                     QByteArray &decryptedData,
                                     bool &signatureFound,
                                     std::vector<GpgME::Signature> &signatures,
                                     bool showWarning,
                                     bool &passphraseError,
                                     bool &actuallyEncrypted,
                                     bool &decryptionStarted,
                                     PartMetaData &partMetaData)
{
    passphraseError = false;
    decryptionStarted = false;
    partMetaData.errorText.clear();
    partMetaData.auditLogError = GpgME::Error();
    partMetaData.auditLog.clear();
    bool bDecryptionOk = false;
    enum { NO_PLUGIN, NOT_INITIALIZED, CANT_DECRYPT }
    cryptPlugError = NO_PLUGIN;

    const Kleo::CryptoBackend::Protocol *cryptProto = cryptoProtocol();

    QString cryptPlugLibName;
    if (cryptProto) {
        cryptPlugLibName = cryptProto->name();
    }

    assert(mSource->decryptMessage());

    const QString errorMsg = i18n("Could not decrypt the data.");
    if (cryptProto) {
        QByteArray ciphertext = data.decodedContent();
#ifdef MARCS_DEBUG
        QString cipherStr = QString::fromLatin1(ciphertext);
        bool cipherIsBinary = (!cipherStr.contains(QStringLiteral("BEGIN ENCRYPTED MESSAGE"), Qt::CaseInsensitive)) &&
                              (!cipherStr.contains(QStringLiteral("BEGIN PGP ENCRYPTED MESSAGE"), Qt::CaseInsensitive)) &&
                              (!cipherStr.contains(QStringLiteral("BEGIN PGP MESSAGE"), Qt::CaseInsensitive));

        dumpToFile("dat_04_reader.encrypted", ciphertext.data(), ciphertext.size());

        QString deb;
        deb =  QLatin1String("\n\nE N C R Y P T E D    D A T A = ");
        if (cipherIsBinary) {
            deb += QLatin1String("[binary data]");
        } else {
            deb += QLatin1String("\"");
            deb += cipherStr;
            deb += QLatin1String("\"");
        }
        deb += "\n\n";
        qCDebug(MESSAGEVIEWER_LOG) << deb;
#endif

        qCDebug(MESSAGEVIEWER_LOG) << "going to call CRYPTPLUG" << cryptPlugLibName;

        // Check whether the memento contains a result from last time:
        const DecryptVerifyBodyPartMemento *m
            = dynamic_cast<DecryptVerifyBodyPartMemento *>(mNodeHelper->bodyPartMemento(&data, "decryptverify"));
        if (!m) {
            Kleo::DecryptVerifyJob *job = cryptProto->decryptVerifyJob();
            if (!job) {
                cryptPlugError = CANT_DECRYPT;
                cryptProto = 0;
            } else {
                DecryptVerifyBodyPartMemento *newM
                    = new DecryptVerifyBodyPartMemento(job, ciphertext);
                if (allowAsync()) {
                    QObject::connect(newM, &CryptoBodyPartMemento::update,
                                     nodeHelper(), &NodeHelper::update);
                    QObject::connect(newM, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), mSource->sourceObject(),
                                     SLOT(update(MessageViewer::Viewer::UpdateMode)));
                    if (newM->start()) {
                        decryptionStarted = true;
                        mHasPendingAsyncJobs = true;
                    } else {
                        m = newM;
                    }
                } else {
                    newM->exec();
                    m = newM;
                }
                mNodeHelper->setBodyPartMemento(&data, "decryptverify", newM);
            }
        } else if (m->isRunning()) {
            decryptionStarted = true;
            mHasPendingAsyncJobs = true;
            m = 0;
        }

        if (m) {
            const QByteArray &plainText = m->plainText();
            const GpgME::DecryptionResult &decryptResult = m->decryptResult();
            const GpgME::VerificationResult &verifyResult = m->verifyResult();
            std::stringstream ss;
            ss << decryptResult << '\n' << verifyResult;
            qCDebug(MESSAGEVIEWER_LOG) << ss.str().c_str();
            signatureFound = verifyResult.signatures().size() > 0;
            signatures = verifyResult.signatures();
            bDecryptionOk = !decryptResult.error();
            passphraseError =  decryptResult.error().isCanceled()
                               || decryptResult.error().code() == GPG_ERR_NO_SECKEY;
            actuallyEncrypted = decryptResult.error().code() != GPG_ERR_NO_DATA;
            partMetaData.errorText = QString::fromLocal8Bit(decryptResult.error().asString());
            partMetaData.auditLogError = m->auditLogError();
            partMetaData.auditLog = m->auditLogAsHtml();
            partMetaData.isEncrypted = actuallyEncrypted;
            if (actuallyEncrypted && decryptResult.numRecipients() > 0) {
                partMetaData.keyId = decryptResult.recipient(0).keyID();
            }

            qCDebug(MESSAGEVIEWER_LOG) << "ObjectTreeParser::decryptMIME: returned from CRYPTPLUG";
            if (bDecryptionOk) {
                decryptedData = plainText;
            } else if (htmlWriter() && showWarning) {
                decryptedData = "<div style=\"font-size:x-large; text-align:center;"
                                "padding:20pt;\">"
                                + errorMsg.toUtf8()
                                + "</div>";
                if (!passphraseError)
                    partMetaData.errorText = i18n("Crypto plug-in \"%1\" could not decrypt the data.", cryptPlugLibName)
                                             + QLatin1String("<br />")
                                             + i18n("Error: %1", partMetaData.errorText);
            }
        }
    }

    if (!cryptProto) {
        decryptedData = "<div style=\"text-align:center; padding:20pt;\">"
                        + errorMsg.toUtf8()
                        + "</div>";
        switch (cryptPlugError) {
        case NOT_INITIALIZED:
            partMetaData.errorText = i18n("Crypto plug-in \"%1\" is not initialized.",
                                          cryptPlugLibName);
            break;
        case CANT_DECRYPT:
            partMetaData.errorText = i18n("Crypto plug-in \"%1\" cannot decrypt messages.",
                                          cryptPlugLibName);
            break;
        case NO_PLUGIN:
            partMetaData.errorText = i18n("No appropriate crypto plug-in was found.");
            break;
        }
    }

    dumpToFile("dat_05_reader.decrypted", decryptedData.data(), decryptedData.size());

    return bDecryptionOk;
}

MessagePart::Ptr ObjectTreeParser::processTextHtmlSubtype(KMime::Content *curNode, ProcessResult &)
{
    HtmlMessagePart::Ptr mp(new HtmlMessagePart(this, curNode, mSource));

    if (curNode->topLevel()->textContent() == curNode  || attachmentStrategy()->defaultDisplay(curNode) == AttachmentStrategy::Inline ||
            showOnlyOneMimePart()) {
    } else {
        // we need the copy of htmlcontent and charset in anycase for the current design of otp.
        //Should be not neeed if otp don't hold any status anymore
        mp->fix();

    }
    return mp;
}

bool ObjectTreeParser::isMailmanMessage(KMime::Content *curNode)
{
    if (!curNode || curNode->head().isEmpty()) {
        return false;
    }
    if (curNode->hasHeader("X-Mailman-Version")) {
        return true;
    }
    if (curNode->hasHeader("X-Mailer")) {
        KMime::Headers::Base *header = curNode->headerByType("X-Mailer");
        if (header->asUnicodeString().contains(QStringLiteral("MAILMAN"), Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool ObjectTreeParser::processMailmanMessage(KMime::Content *curNode)
{
    const QString str = QString::fromLatin1(curNode->decodedContent());

    //###
    const QLatin1String delim1("--__--__--\n\nMessage:");
    const QLatin1String delim2("--__--__--\r\n\r\nMessage:");
    const QLatin1String delimZ2("--__--__--\n\n_____________");
    const QLatin1String delimZ1("--__--__--\r\n\r\n_____________");
    QString partStr, digestHeaderStr;
    int thisDelim = str.indexOf(delim1, Qt::CaseInsensitive);
    if (thisDelim == -1) {
        thisDelim = str.indexOf(delim2, Qt::CaseInsensitive);
    }
    if (thisDelim == -1) {
        return false;
    }

    int nextDelim = str.indexOf(delim1, thisDelim + 1, Qt::CaseInsensitive);
    if (-1 == nextDelim) {
        nextDelim = str.indexOf(delim2, thisDelim + 1, Qt::CaseInsensitive);
    }
    if (-1 == nextDelim) {
        nextDelim = str.indexOf(delimZ1, thisDelim + 1, Qt::CaseInsensitive);
    }
    if (-1 == nextDelim) {
        nextDelim = str.indexOf(delimZ2, thisDelim + 1, Qt::CaseInsensitive);
    }
    if (nextDelim < 0) {
        return false;
    }

    //if ( curNode->mRoot )
    //  curNode = curNode->mRoot;

    // at least one message found: build a mime tree
    digestHeaderStr = QStringLiteral("Content-Type: text/plain\nContent-Description: digest header\n\n");
    digestHeaderStr += str.midRef(0, thisDelim);
    createAndParseTempNode(mTopLevelContent, digestHeaderStr.toLatin1(), "Digest Header");
    //mReader->queueHtml("<br><hr><br>");
    // temporarily change curent node's Content-Type
    // to get our embedded RfC822 messages properly inserted
    curNode->contentType()->setMimeType("multipart/digest");
    while (-1 < nextDelim) {
        int thisEoL = str.indexOf(QLatin1String("\nMessage:"), thisDelim, Qt::CaseInsensitive);
        if (-1 < thisEoL) {
            thisDelim = thisEoL + 1;
        } else {
            thisEoL = str.indexOf(QLatin1String("\n_____________"), thisDelim, Qt::CaseInsensitive);
            if (-1 < thisEoL) {
                thisDelim = thisEoL + 1;
            }
        }
        thisEoL = str.indexOf(QLatin1Char('\n'), thisDelim);
        if (-1 < thisEoL) {
            thisDelim = thisEoL + 1;
        } else {
            thisDelim = thisDelim + 1;
        }
        //while( thisDelim < cstr.size() && '\n' == cstr[thisDelim] )
        //  ++thisDelim;

        partStr = QStringLiteral("Content-Type: message/rfc822\nContent-Description: embedded message\n\n");
        partStr += QLatin1String("Content-Type: text/plain\n");
        partStr += str.midRef(thisDelim, nextDelim - thisDelim);
        QString subject = QStringLiteral("embedded message");
        QString subSearch = QStringLiteral("\nSubject:");
        int subPos = partStr.indexOf(subSearch, 0, Qt::CaseInsensitive);
        if (-1 < subPos) {
            subject = partStr.mid(subPos + subSearch.length());
            thisEoL = subject.indexOf(QLatin1Char('\n'));
            if (-1 < thisEoL) {
                subject.truncate(thisEoL);
            }
        }
        qCDebug(MESSAGEVIEWER_LOG) << "        embedded message found: \"" << subject;
        createAndParseTempNode(mTopLevelContent, partStr.toLatin1(), subject.toLatin1());
        //mReader->queueHtml("<br><hr><br>");
        thisDelim = nextDelim + 1;
        nextDelim = str.indexOf(delim1, thisDelim, Qt::CaseInsensitive);
        if (-1 == nextDelim) {
            nextDelim = str.indexOf(delim2, thisDelim, Qt::CaseInsensitive);
        }
        if (-1 == nextDelim) {
            nextDelim = str.indexOf(delimZ1, thisDelim, Qt::CaseInsensitive);
        }
        if (-1 == nextDelim) {
            nextDelim = str.indexOf(delimZ2, thisDelim, Qt::CaseInsensitive);
        }
    }
    // reset curent node's Content-Type
    curNode->contentType()->setMimeType("text/plain");
    int thisEoL = str.indexOf(QLatin1String("_____________"), thisDelim);
    if (-1 < thisEoL) {
        thisDelim = thisEoL;
        thisEoL = str.indexOf(QLatin1Char('\n'), thisDelim);
        if (-1 < thisEoL) {
            thisDelim = thisEoL + 1;
        }
    } else {
        thisDelim = thisDelim + 1;
    }
    partStr = QStringLiteral("Content-Type: text/plain\nContent-Description: digest footer\n\n");
    partStr += str.midRef(thisDelim);
    createAndParseTempNode(mTopLevelContent, partStr.toLatin1(), "Digest Footer");
    return true;
}

void ObjectTreeParser::extractNodeInfos(KMime::Content *curNode, bool isFirstTextPart)
{
    if (isFirstTextPart) {
        mPlainTextContent += curNode->decodedText();
        mPlainTextContentCharset += NodeHelper::charset(curNode);
    }
}

MessagePart::Ptr ObjectTreeParser::processTextPlainSubtype(KMime::Content *curNode, ProcessResult &result)
{
    const bool isFirstTextPart = (curNode->topLevel()->textContent() == curNode);

    if (!isFirstTextPart && attachmentStrategy()->defaultDisplay(curNode) != AttachmentStrategy::Inline &&
            !showOnlyOneMimePart()) {
        return  MessagePart::Ptr();
    }

    extractNodeInfos(curNode, isFirstTextPart);

    QString label = NodeHelper::fileName(curNode);

    const bool bDrawFrame = !isFirstTextPart
                            && !showOnlyOneMimePart()
                            && !label.isEmpty();
    const QString fileName = mNodeHelper->writeNodeToTempFile(curNode);

    // process old style not-multipart Mailman messages to
    // enable verification of the embedded messages' signatures
    //if (!isMailmanMessage(curNode) ||
    //        !processMailmanMessage(curNode)) {

        TextMessagePart::Ptr mp(new TextMessagePart(this, curNode, bDrawFrame, !fileName.isEmpty(), mSource->decryptMessage()));

        result.setInlineSignatureState(mp->signatureState());
        result.setInlineEncryptionState(mp->encryptionState());

        if (isFirstTextPart) {
            mPlainTextContent = mp->text();
        }

        mNodeHelper->setNodeDisplayedEmbedded(curNode, true);
    //}

    return mp;
}

MessagePart::Ptr ObjectTreeParser::processMultiPartMixedSubtype(KMime::Content *node, ProcessResult &)
{
    KMime::Content *child = MessageCore::NodeHelper::firstChild(node);
    if (!child) {
        return MessagePart::Ptr();
    }

    // normal treatment of the parts in the mp/mixed container
    MimeMessagePart::Ptr mp(new MimeMessagePart(this, child, false));
    return mp;
}

MessagePart::Ptr ObjectTreeParser::processMultiPartAlternativeSubtype(KMime::Content *node, ProcessResult &)
{
    KMime::Content *child = MessageCore::NodeHelper::firstChild(node);
    if (!child) {
         return MessagePart::Ptr();
    }

    KMime::Content *dataHtml = findType(child, "text/html", false, true);
    KMime::Content *dataPlain = findType(child, "text/plain", false, true);

    if (!dataHtml) {
        // If we didn't find the HTML part as the first child of the multipart/alternative, it might
        // be that this is a HTML message with images, and text/plain and multipart/related are the
        // immediate children of this multipart/alternative node.
        // In this case, the HTML node is a child of multipart/related.
        dataHtml = findType(child, "multipart/related", false, true);

        // Still not found? Stupid apple mail actually puts the attachments inside of the
        // multipart/alternative, which is wrong. Therefore we also have to look for multipart/mixed
        // here.
        // Do this only when prefering HTML mail, though, since otherwise the attachments are hidden
        // when displaying plain text.
        if (!dataHtml && mSource->htmlMail()) {
            dataHtml = findType(child, "multipart/mixed", false, true);
        }
    }

    if (dataPlain || dataHtml) {
        AlternativeMessagePart::Ptr mp(new AlternativeMessagePart(this, dataPlain, dataHtml));

        if ((mSource->htmlMail() && dataHtml) ||
            (dataHtml && dataPlain && dataPlain->body().isEmpty())) {
            if (dataPlain) {
                mNodeHelper->setNodeProcessed(dataPlain, false);
            }
            mSource->setHtmlMode(Util::MultipartHtml);
            mp->setViewHtml(true);
        }

        if (!mSource->htmlMail() && dataPlain) {
            mNodeHelper->setNodeProcessed(dataHtml, false);
            mSource->setHtmlMode(Util::MultipartPlain);
            mp->setViewHtml(false);
        }
        return mp;
    }

    MimeMessagePart::Ptr mp(new MimeMessagePart(this, child, false));
    return mp;
}

MessagePart::Ptr ObjectTreeParser::processMultiPartSignedSubtype(KMime::Content *node, ProcessResult &)
{
    KMime::Content *signedData = MessageCore::NodeHelper::firstChild(node);
    assert(signedData);
    if (node->contents().size() != 2) {
        qCDebug(MESSAGEVIEWER_LOG) << "mulitpart/signed must have exactly two child parts!" << endl
                                   << "processing as multipart/mixed";

        return MessagePart::Ptr(new MimeMessagePart(this, signedData, false));
    }

    KMime::Content *signature = node->contents().at(1);
    assert(signature);

    QString protocolContentType = node->contentType()->parameter(QStringLiteral("protocol")).toLower();
    const QString signatureContentType = QLatin1String(signature->contentType()->mimeType().toLower());
    if (protocolContentType.isEmpty()) {
        qCWarning(MESSAGEVIEWER_LOG) << "Message doesn't set the protocol for the multipart/signed content-type, "
                                     "using content-type of the signature:" << signatureContentType;
        protocolContentType = signatureContentType;
    }

    const Kleo::CryptoBackend::Protocol *protocol = 0;
    if (protocolContentType == QLatin1String("application/pkcs7-signature") ||
            protocolContentType == QLatin1String("application/x-pkcs7-signature")) {
        protocol = Kleo::CryptoBackendFactory::instance()->smime();
    } else if (protocolContentType == QLatin1String("application/pgp-signature") ||
               protocolContentType == QLatin1String("application/x-pgp-signature")) {
        protocol = Kleo::CryptoBackendFactory::instance()->openpgp();
    }

    if (!protocol) {
        return MessagePart::Ptr(new MimeMessagePart(this, signedData, false));
    }

    mNodeHelper->setNodeProcessed(signature, true);

    CryptoProtocolSaver saver(this, protocol);
    mNodeHelper->setSignatureState(node, KMMsgFullySigned);

    const QByteArray cleartext = KMime::LFtoCRLF(signedData->encodedContent());
    const QTextCodec *aCodec(codecFor(signedData));

    CryptoMessagePart::Ptr mp(new CryptoMessagePart(this,
                              aCodec->toUnicode(cleartext), cryptoProtocol(),
                              NodeHelper::fromAsString(node), signature));
    PartMetaData *messagePart(mp->partMetaData());
    messagePart->isSigned = true;

    if (cryptoProtocol()) {
        mp->startVerificationDetached(cleartext, signedData, signature->decodedContent());
    } else {
        messagePart->auditLogError = GpgME::Error(GPG_ERR_NOT_IMPLEMENTED);
    }

    return mp;
}

MessagePart::Ptr ObjectTreeParser::processMultiPartEncryptedSubtype(KMime::Content *node, ProcessResult &result)
{
    KMime::Content *child = MessageCore::NodeHelper::firstChild(node);
    if (!child) {
        Q_ASSERT(false);
        return MessagePart::Ptr();
    }

    const Kleo::CryptoBackend::Protocol *useThisCryptProto = Q_NULLPTR;

    /*
    ATTENTION: This code is to be replaced by the new 'auto-detect' feature. --------------------------------------
    */
    KMime::Content *data = findType(child, "application/octet-stream", false, true);
    if (data) {
        useThisCryptProto = Kleo::CryptoBackendFactory::instance()->openpgp();
    }
    if (!data) {
        data = findType(child, "application/pkcs7-mime", false, true);
        if (data) {
            useThisCryptProto = Kleo::CryptoBackendFactory::instance()->smime();
        }
    }
    /*
    ---------------------------------------------------------------------------------------------------------------
    */

    if (!data) {
        return MessagePart::Ptr(new MimeMessagePart(this, child, false));
    }

    CryptoProtocolSaver cpws(this, useThisCryptProto);

    KMime::Content *dataChild = MessageCore::NodeHelper::firstChild(data);
    if (dataChild) {
        Q_ASSERT(false);
        return MessagePart::Ptr(new MimeMessagePart(this, dataChild, false));
    }

    mNodeHelper->setEncryptionState(node, KMMsgFullyEncrypted);

    CryptoMessagePart::Ptr mp(new CryptoMessagePart(this,
                              data->decodedText(), Kleo::CryptoBackendFactory::instance()->openpgp(),
                              NodeHelper::fromAsString(data), node));

    PartMetaData *messagePart(mp->partMetaData());
    if (!mSource->decryptMessage()) {
        mNodeHelper->setNodeProcessed(data, false);  // Set the data node to done to prevent it from being processed
    } else if (KMime::Content *newNode = mNodeHelper->decryptedNodeForContent(data)) {
        // if we already have a decrypted node for this encrypted node, don't do the decryption again
        return MessagePart::Ptr(new MimeMessagePart(this, newNode, mShowOnlyOneMimePart));
    } else {
        mp->startDecryption(data);

        qCDebug(MESSAGEVIEWER_LOG) << "decrypted, signed?:" << messagePart->isSigned;

        if (!messagePart->inProgress) {
            mNodeHelper->setNodeProcessed(data, false);   // Set the data node to done to prevent it from being processed
            if (messagePart->isDecryptable && messagePart->isSigned) {
                // Note: Multipart/Encrypted might also be signed
                //       without encapsulating a nicely formatted
                //       ~~~~~~~                 Multipart/Signed part.
                //                               (see RFC 3156 --> 6.2)
                // In this case we paint a _2nd_ frame inside the
                // encryption frame, but we do _not_ show a respective
                // encapsulated MIME part in the Mime Tree Viewer
                // since we do want to show the _true_ structure of the
                // message there - not the structure that the sender's
                // MUA 'should' have sent.  :-D       (khz, 12.09.2002)

                mNodeHelper->setSignatureState(node, KMMsgFullySigned);
                qCDebug(MESSAGEVIEWER_LOG) << "setting FULLY SIGNED to:" << node;
            }
        }
    }
    return mp;
}

MessagePart::Ptr ObjectTreeParser::processApplicationPkcs7MimeSubtype(KMime::Content *node, ProcessResult &result)
{
    if (node->head().isEmpty()) {
        return MessagePart::Ptr();
    }

    const Kleo::CryptoBackend::Protocol *smimeCrypto = Kleo::CryptoBackendFactory::instance()->smime();
    if (!smimeCrypto) {
        return MessagePart::Ptr();
    }

    const QString smimeType = node->contentType()->parameter(QStringLiteral("smime-type")).toLower();

    if (smimeType == QLatin1String("certs-only")) {
        result.setNeverDisplayInline(true);

        CertMessagePart::Ptr mp(new CertMessagePart(this, node, smimeCrypto, MessageViewer::MessageViewerSettings::self()->autoImportKeys()));
        return mp;
    }

    CryptoProtocolSaver cpws(this, smimeCrypto);

    bool isSigned      = (smimeType == QLatin1String("signed-data"));
    bool isEncrypted   = (smimeType == QLatin1String("enveloped-data"));

    // Analyze "signTestNode" node to find/verify a signature.
    // If zero this verification was successfully done after
    // decrypting via recursion by insertAndParseNewChildNode().
    KMime::Content *signTestNode = isEncrypted ? 0 : node;

    // We try decrypting the content
    // if we either *know* that it is an encrypted message part
    // or there is neither signed nor encrypted parameter.
    CryptoMessagePart::Ptr mp;
    if (!isSigned) {
        if (isEncrypted) {
            qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime     ==      S/MIME TYPE: enveloped (encrypted) data";
        } else {
            qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  type unknown  -  enveloped (encrypted) data ?";
        }

        mp = CryptoMessagePart::Ptr(new CryptoMessagePart(this,
                             node->decodedText(), cryptoProtocol(),
                             NodeHelper::fromAsString(node), node));

        PartMetaData *messagePart(mp->partMetaData());
        if (!mSource->decryptMessage()) {
            isEncrypted = true;
            signTestNode = 0; // PENDING(marc) to be abs. sure, we'd need to have to look at the content
        } else {
            mp->startDecryption();
            if (messagePart->isDecryptable) {
                qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  encryption found  -  enveloped (encrypted) data !";
                isEncrypted = true;
                mNodeHelper->setEncryptionState(node, KMMsgFullyEncrypted);
                if (messagePart->isSigned) {
                    mNodeHelper->setSignatureState(node, KMMsgFullySigned);
                }
                signTestNode = 0;

            } else {
                // decryption failed, which could be because the part was encrypted but
                // decryption failed, or because we didn't know if it was encrypted, tried,
                // and failed. If the message was not actually encrypted, we continue
                // assuming it's signed
                if (mp->mPassphraseError || (smimeType.isEmpty() && messagePart->isEncrypted)) {
                    isEncrypted = true;
                    signTestNode = 0;
                }

                if (isEncrypted) {
                    qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  ERROR: COULD NOT DECRYPT enveloped data !";
                } else {
                    qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  NO encryption found";
                }
            }
        }

        if (isEncrypted) {
            mNodeHelper->setEncryptionState(node, KMMsgFullyEncrypted);
        }
    }

    // We now try signature verification if necessarry.
    if (signTestNode) {
        if (isSigned) {
            qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime     ==      S/MIME TYPE: opaque signed data";
        } else {
            qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  type unknown  -  opaque signed data ?";
        }

        const QTextCodec *aCodec(codecFor(signTestNode));
        const QByteArray signaturetext = signTestNode->decodedContent();
        mp = CryptoMessagePart::Ptr(new CryptoMessagePart(this,
                             aCodec->toUnicode(signaturetext), cryptoProtocol(),
                             NodeHelper::fromAsString(node), signTestNode));

        PartMetaData *messagePart(mp->partMetaData());
        if (cryptoProtocol()) {
            mp->startVerificationDetached(signaturetext, 0, QByteArray());
        } else {
            messagePart->auditLogError = GpgME::Error(GPG_ERR_NOT_IMPLEMENTED);
        }

        if (messagePart->isSigned) {
            if (!isSigned) {
                qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  signature found  -  opaque signed data !";
                isSigned = true;
            }

            mNodeHelper->setSignatureState(signTestNode, KMMsgFullySigned);
            if (signTestNode != node) {
                mNodeHelper->setSignatureState(node, KMMsgFullySigned);
            }
        } else {
            qCDebug(MESSAGEVIEWER_LOG) << "pkcs7 mime  -  NO signature found   :-(";
        }
    }

    return mp;
}

void ObjectTreeParser::writePartIcon(KMime::Content *msgPart, bool inlineImage)
{
    if (!htmlWriter() || !msgPart) {
        return;
    }

    const QString name = msgPart->contentType()->name();
    QString label = name.isEmpty() ? NodeHelper::fileName(msgPart) : name;
    if (label.isEmpty()) {
        label = i18nc("display name for an unnamed attachment", "Unnamed");
    }
    label = StringUtil::quoteHtmlChars(label, true);

    QString comment = msgPart->contentDescription()->asUnicodeString();
    comment = StringUtil::quoteHtmlChars(comment, true);
    if (label == comment) {
        comment.clear();
    }

    QString href = mNodeHelper->asHREF(msgPart, QStringLiteral("body"));

    if (inlineImage) {
        const QString fileName = mNodeHelper->writeNodeToTempFile(msgPart);
        // show the filename of the image below the embedded image
        htmlWriter()->queue(QLatin1String("<div><a href=\"") + href + QLatin1String("\">"
                            "<img src=\"file:///") + fileName + QLatin1String("\" border=\"0\" style=\"max-width: 100%\"/></a>"
                                    "</div>"
                                    "<div><a href=\"") + href + QLatin1String("\">") + label + QLatin1String("</a>"
                                            "</div>"
                                            "<div>") + comment + QLatin1String("</div><br/>"));
    } else {
        // show the filename next to the image
        const QString iconName = mNodeHelper->iconName(msgPart);
        if (iconName.right(14) == QLatin1String("mime_empty.png")) {
            mNodeHelper->magicSetType(msgPart);
            //iconName = mNodeHelper->iconName( msgPart );
        }
        htmlWriter()->queue(QLatin1String("<div><a href=\"") + href + QLatin1String("\"><img src=\"file:///") +
                            iconName + QLatin1String("\" border=\"0\" style=\"max-width: 100%\" alt=\"\"/>") + label +
                            QLatin1String("</a></div>"
                                          "<div>") + comment + QLatin1String("</div><br/>"));
    }
}

static const int SIG_FRAME_COL_UNDEF = 99;
#define SIG_FRAME_COL_RED    -1
#define SIG_FRAME_COL_YELLOW  0
#define SIG_FRAME_COL_GREEN   1
QString ObjectTreeParser::sigStatusToString(const Kleo::CryptoBackend::Protocol *cryptProto,
        int status_code,
        GpgME::Signature::Summary summary,
        int &frameColor,
        bool &showKeyInfos)
{
    // note: At the moment frameColor and showKeyInfos are
    //       used for CMS only but not for PGP signatures
    // pending(khz): Implement usage of these for PGP sigs as well.
    showKeyInfos = true;
    QString result;
    if (cryptProto) {
        if (cryptProto == Kleo::CryptoBackendFactory::instance()->openpgp()) {
            // process enum according to it's definition to be read in
            // GNU Privacy Guard CVS repository /gpgme/gpgme/gpgme.h
            switch (status_code) {
            case 0: // GPGME_SIG_STAT_NONE
                result = i18n("Error: Signature not verified");
                break;
            case 1: // GPGME_SIG_STAT_GOOD
                result = i18n("Good signature");
                break;
            case 2: // GPGME_SIG_STAT_BAD
                result = i18n("<b>Bad</b> signature");
                break;
            case 3: // GPGME_SIG_STAT_NOKEY
                result = i18n("No public key to verify the signature");
                break;
            case 4: // GPGME_SIG_STAT_NOSIG
                result = i18n("No signature found");
                break;
            case 5: // GPGME_SIG_STAT_ERROR
                result = i18n("Error verifying the signature");
                break;
            case 6: // GPGME_SIG_STAT_DIFF
                result = i18n("Different results for signatures");
                break;
            /* PENDING(khz) Verify exact meaning of the following values:
            case 7: // GPGME_SIG_STAT_GOOD_EXP
            return i18n("Signature certificate is expired");
            break;
            case 8: // GPGME_SIG_STAT_GOOD_EXPKEY
            return i18n("One of the certificate's keys is expired");
            break;
            */
            default:
                result.clear();   // do *not* return a default text here !
                break;
            }
        } else if (cryptProto == Kleo::CryptoBackendFactory::instance()->smime()) {
            // process status bits according to SigStatus_...
            // definitions in kdenetwork/libkdenetwork/cryptplug.h

            if (summary == GpgME::Signature::None) {
                result = i18n("No status information available.");
                frameColor = SIG_FRAME_COL_YELLOW;
                showKeyInfos = false;
                return result;
            }

            if (summary & GpgME::Signature::Valid) {
                result = i18n("Good signature.");
                // Note:
                // Here we are work differently than KMail did before!
                //
                // The GOOD case ( == sig matching and the complete
                // certificate chain was verified and is valid today )
                // by definition does *not* show any key
                // information but just states that things are OK.
                //           (khz, according to LinuxTag 2002 meeting)
                frameColor = SIG_FRAME_COL_GREEN;
                showKeyInfos = false;
                return result;
            }

            // we are still there?  OK, let's test the different cases:

            // we assume green, test for yellow or red (in this order!)
            frameColor = SIG_FRAME_COL_GREEN;
            QString result2;
            if (summary & GpgME::Signature::KeyExpired) {
                // still is green!
                result2 += i18n("One key has expired.");
            }
            if (summary & GpgME::Signature::SigExpired) {
                // and still is green!
                result2 += i18n("The signature has expired.");
            }

            // test for yellow:
            if (summary & GpgME::Signature::KeyMissing) {
                result2 += i18n("Unable to verify: key missing.");
                // if the signature certificate is missing
                // we cannot show information on it
                showKeyInfos = false;
                frameColor = SIG_FRAME_COL_YELLOW;
            }
            if (summary & GpgME::Signature::CrlMissing) {
                result2 += i18n("CRL not available.");
                frameColor = SIG_FRAME_COL_YELLOW;
            }
            if (summary & GpgME::Signature::CrlTooOld) {
                result2 += i18n("Available CRL is too old.");
                frameColor = SIG_FRAME_COL_YELLOW;
            }
            if (summary & GpgME::Signature::BadPolicy) {
                result2 += i18n("A policy was not met.");
                frameColor = SIG_FRAME_COL_YELLOW;
            }
            if (summary & GpgME::Signature::SysError) {
                result2 += i18n("A system error occurred.");
                // if a system error occurred
                // we cannot trust any information
                // that was given back by the plug-in
                showKeyInfos = false;
                frameColor = SIG_FRAME_COL_YELLOW;
            }

            // test for red:
            if (summary & GpgME::Signature::KeyRevoked) {
                // this is red!
                result2 += i18n("One key has been revoked.");
                frameColor = SIG_FRAME_COL_RED;
            }
            if (summary & GpgME::Signature::Red) {
                if (result2.isEmpty())
                    // Note:
                    // Here we are work differently than KMail did before!
                    //
                    // The BAD case ( == sig *not* matching )
                    // by definition does *not* show any key
                    // information but just states that things are BAD.
                    //
                    // The reason for this: In this case ALL information
                    // might be falsificated, we can NOT trust the data
                    // in the body NOT the signature - so we don't show
                    // any key/signature information at all!
                    //         (khz, according to LinuxTag 2002 meeting)
                {
                    showKeyInfos = false;
                }
                frameColor = SIG_FRAME_COL_RED;
            } else {
                result.clear();
            }

            if (SIG_FRAME_COL_GREEN == frameColor) {
                result = i18n("Good signature.");
            } else if (SIG_FRAME_COL_RED == frameColor) {
                result = i18n("<b>Bad</b> signature.");
            } else {
                result.clear();
            }

            if (!result2.isEmpty()) {
                if (!result.isEmpty()) {
                    result.append(QLatin1String("<br />"));
                }
                result.append(result2);
            }
        }
        /*
        // add i18n support for 3rd party plug-ins here:
        else if ( cryptPlug->libName().contains( "yetanotherpluginname", Qt::CaseInsensitive )) {

        }
        */
    }
    return result;
}

static QString writeSimpleSigstatHeader(const PartMetaData &block, bool printing)
{
    QString html;
    html += QLatin1String("<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%\"><tr><td>");

    if (block.signClass == QLatin1String("signErr")) {
        html += i18n("Invalid signature.");
    } else if (block.signClass == QLatin1String("signOkKeyBad")
               || block.signClass == QLatin1String("signWarn")) {
        html += i18n("Not enough information to check signature validity.");
    } else if (block.signClass == QLatin1String("signOkKeyOk")) {

        QString addr;
        if (!block.signerMailAddresses.isEmpty()) {
            addr = block.signerMailAddresses.first();
        }

        QString name = addr;
        if (name.isEmpty()) {
            name = block.signer;
        }

        if (addr.isEmpty()) {
            html += i18n("Signature is valid.");
        } else {
            html += i18n("Signed by <a href=\"mailto:%1\">%2</a>.", addr, name);
        }

    } else {
        // should not happen
        html += i18n("Unknown signature state");
    }
    html += QLatin1String("</td>");
    if (!printing) {
        html += QLatin1String("<td align=\"right\">");
        html += QLatin1String("<a href=\"kmail:showSignatureDetails\">");
        html += i18n("Show Details");
        html += QLatin1String("</a></td>");
    }
    html += QLatin1String("</tr></table>");
    return html;
}

static QString beginVerboseSigstatHeader()
{
    return QStringLiteral("<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%\"><tr><td rowspan=\"2\">");
}

static QString makeShowAuditLogLink(const GpgME::Error &err, const QString &auditLog)
{
    // more or less the same as
    // kleopatra/utils/auditlog.cpp:formatLink(), so any bug fixed here
    // equally applies there:
    if (const unsigned int code = err.code()) {
        if (code == GPG_ERR_NOT_IMPLEMENTED) {
            qCDebug(MESSAGEVIEWER_LOG) << "not showing link (not implemented)";
            return QString();
        } else if (code == GPG_ERR_NO_DATA) {
            qCDebug(MESSAGEVIEWER_LOG) << "not showing link (not available)";
            return i18n("No Audit Log available");
        } else {
            return i18n("Error Retrieving Audit Log: %1", QString::fromLocal8Bit(err.asString()));
        }
    }

    if (!auditLog.isEmpty()) {
        QUrl url;
        url.setScheme(QStringLiteral("kmail"));
        url.setPath(QStringLiteral("showAuditLog"));
        QUrlQuery urlquery(url);
        urlquery.addQueryItem(QStringLiteral("log"), auditLog);
        url.setQuery(urlquery);
        return QLatin1String("<a href=\"") + url.toDisplayString() + QLatin1String("\">") + i18nc("The Audit Log is a detailed error log from the gnupg backend", "Show Audit Log") + QLatin1String("</a>");
    }

    return QString();
}

static QString endVerboseSigstatHeader(const PartMetaData &pmd)
{
    QString html;
    html += QLatin1String("</td><td align=\"right\" valign=\"top\" nowrap=\"nowrap\">");
    html += QLatin1String("<a href=\"kmail:hideSignatureDetails\">");
    html += i18n("Hide Details");
    html += QLatin1String("</a></td></tr>");
    html += QLatin1String("<tr><td align=\"right\" valign=\"bottom\" nowrap=\"nowrap\">");
    html += makeShowAuditLogLink(pmd.auditLogError, pmd.auditLog);
    html += QLatin1String("</td></tr></table>");
    return html;
}

QString ObjectTreeParser::writeSigstatHeader(PartMetaData &block,
        const Kleo::CryptoBackend::Protocol *cryptProto,
        const QString &fromAddress,
        KMime::Content *node)
{
    const bool isSMIME = cryptProto && (cryptProto == Kleo::CryptoBackendFactory::instance()->smime());
    QString signer = block.signer;

    QString htmlStr, simpleHtmlStr;
    const QString dir = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");
    QString cellPadding(QStringLiteral("cellpadding=\"1\""));

    if (block.isEncapsulatedRfc822Message) {
        htmlStr += QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" class=\"rfc822\">"
                   "<tr class=\"rfc822H\"><td dir=\"") + dir + QLatin1String("\">");
        if (node) {
            htmlStr += QLatin1String("<a href=\"") + mNodeHelper->asHREF(node, QStringLiteral("body")) + QLatin1String("\">")
                       + i18n("Encapsulated message") + QLatin1String("</a>");
        } else {
            htmlStr += i18n("Encapsulated message");
        }
        htmlStr += QLatin1String("</td></tr><tr class=\"rfc822B\"><td>");
    }

    if (block.isEncrypted) {
        htmlStr += QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" class=\"encr\">"
                   "<tr class=\"encrH\"><td dir=\"") + dir + QLatin1String("\">");
        if (block.inProgress) {
            htmlStr += i18n("Please wait while the message is being decrypted...");
        } else if (block.isDecryptable) {
            htmlStr += i18n("Encrypted message");
        } else {
            htmlStr += i18n("Encrypted message (decryption not possible)");
            if (!block.errorText.isEmpty()) {
                htmlStr += QLatin1String("<br />") + i18n("Reason: %1", block.errorText);
            }
        }
        htmlStr += QLatin1String("</td></tr><tr class=\"encrB\"><td>");
    }

    if (block.isSigned && block.inProgress) {
        block.signClass = QStringLiteral("signInProgress");
        htmlStr += QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" class=\"signInProgress\">"
                   "<tr class=\"signInProgressH\"><td dir=\"") + dir + QLatin1String("\">");
        htmlStr += i18n("Please wait while the signature is being verified...");
        htmlStr += QLatin1String("</td></tr><tr class=\"signInProgressB\"><td>");
    }

    simpleHtmlStr = htmlStr;

    if (block.isSigned && !block.inProgress) {
        QStringList &blockAddrs(block.signerMailAddresses);
        // note: At the moment frameColor and showKeyInfos are
        //       used for CMS only but not for PGP signatures
        // pending(khz): Implement usage of these for PGP sigs as well.
        int frameColor = SIG_FRAME_COL_UNDEF;
        bool showKeyInfos;
        bool onlyShowKeyURL = false;
        bool cannotCheckSignature = true;
        QString statusStr = sigStatusToString(cryptProto,
                                              block.status_code,
                                              block.sigSummary,
                                              frameColor,
                                              showKeyInfos);
        // if needed fallback to english status text
        // that was reported by the plugin
        if (statusStr.isEmpty()) {
            statusStr = block.status;
        }
        if (block.technicalProblem) {
            frameColor = SIG_FRAME_COL_YELLOW;
        }

        switch (frameColor) {
        case SIG_FRAME_COL_RED:
            cannotCheckSignature = false;
            break;
        case SIG_FRAME_COL_YELLOW:
            cannotCheckSignature = true;
            break;
        case SIG_FRAME_COL_GREEN:
            cannotCheckSignature = false;
            break;
        }

        // compose the string for displaying the key ID
        // either as URL or not linked (for unknown crypto)
        // note: Once we can start PGP key manager programs
        //       from within KMail we could change this and
        //       always show the URL.    (khz, 2002/06/27)
        QString startKeyHREF;
        QString keyWithWithoutURL;
        if (cryptProto) {
            startKeyHREF =
                QStringLiteral("<a href=\"kmail:showCertificate#%1 ### %2 ### %3\">")
                .arg(cryptProto->displayName(),
                     cryptProto->name(),
                     QString::fromLatin1(block.keyId));

            keyWithWithoutURL =
                QStringLiteral("%1%2</a>").arg(startKeyHREF, QString::fromLatin1(QByteArray(QByteArray("0x") + block.keyId)));
        } else {
            keyWithWithoutURL = QLatin1String("0x") + QString::fromUtf8(block.keyId);
        }

        // temporary hack: always show key information!
        showKeyInfos = true;

        // Sorry for using 'black' as null color but .isValid()
        // checking with QColor default c'tor did not work for
        // some reason.
        if (isSMIME && (SIG_FRAME_COL_UNDEF != frameColor)) {

            // new frame settings for CMS:
            // beautify the status string
            if (!statusStr.isEmpty()) {
                statusStr.prepend(QLatin1String("<i>"));
                statusStr.append(QLatin1String("</i>"));
            }

            // special color handling: S/MIME uses only green/yellow/red.
            switch (frameColor) {
            case SIG_FRAME_COL_RED:
                block.signClass = QStringLiteral("signErr");//"signCMSRed";
                onlyShowKeyURL = true;
                break;
            case SIG_FRAME_COL_YELLOW:
                if (block.technicalProblem) {
                    block.signClass = QStringLiteral("signWarn");
                } else {
                    block.signClass = QStringLiteral("signOkKeyBad");    //"signCMSYellow";
                }
                break;
            case SIG_FRAME_COL_GREEN:
                block.signClass = QStringLiteral("signOkKeyOk");//"signCMSGreen";
                // extra hint for green case
                // that email addresses in DN do not match fromAddress
                QString greenCaseWarning;
                QString msgFrom(KEmailAddress::extractEmailAddress(fromAddress));
                QString certificate;
                if (block.keyId.isEmpty()) {
                    certificate = i18n("certificate");
                } else {
                    certificate = startKeyHREF + i18n("certificate") + QLatin1String("</a>");
                }
                if (!blockAddrs.empty()) {
                    if (!blockAddrs.contains(msgFrom, Qt::CaseInsensitive)) {
                        greenCaseWarning =
                            QLatin1String("<u>") +
                            i18nc("Start of warning message."
                                  , "Warning:") +
                            QLatin1String("</u> ") +
                            i18n("Sender's mail address is not stored "
                                 "in the %1 used for signing.", certificate) +
                            QLatin1String("<br />") +
                            i18n("sender: ") +
                            msgFrom +
                            QLatin1String("<br />") +
                            i18n("stored: ");
                        // We cannot use Qt's join() function here but
                        // have to join the addresses manually to
                        // extract the mail addresses (without '<''>')
                        // before including it into our string:
                        bool bStart = true;
                        for (QStringList::ConstIterator it = blockAddrs.constBegin();
                                it != blockAddrs.constEnd(); ++it) {
                            if (!bStart) {
                                greenCaseWarning.append(QLatin1String(", <br />&nbsp; &nbsp;"));
                            }
                            bStart = false;
                            greenCaseWarning.append(KEmailAddress::extractEmailAddress(*it));
                        }
                    }
                } else {
                    greenCaseWarning =
                        QLatin1String("<u>") +
                        i18nc("Start of warning message.", "Warning:") +
                        QLatin1String("</u> ") +
                        i18n("No mail address is stored in the %1 used for signing, "
                             "so we cannot compare it to the sender's address %2.",
                             certificate,
                             msgFrom);
                }
                if (!greenCaseWarning.isEmpty()) {
                    if (!statusStr.isEmpty()) {
                        statusStr.append(QLatin1String("<br />&nbsp;<br />"));
                    }
                    statusStr.append(greenCaseWarning);
                }
                break;
            }

            QString frame = QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" "
                            "class=\"") + block.signClass + QLatin1String("\">"
                                    "<tr class=\"") + block.signClass + QLatin1String("H\"><td dir=\"") + dir + QLatin1String("\">");
            htmlStr += frame + beginVerboseSigstatHeader();
            simpleHtmlStr += frame;
            simpleHtmlStr += writeSimpleSigstatHeader(block, mPrinting);
            if (block.technicalProblem) {
                htmlStr += block.errorText;
            } else if (showKeyInfos) {
                if (cannotCheckSignature) {
                    htmlStr += i18n("Not enough information to check "
                                    "signature. %1",
                                    keyWithWithoutURL);
                } else {

                    if (block.signer.isEmpty()) {
                        signer.clear();
                    } else {
                        if (!blockAddrs.empty()) {
                            const QUrl address = KEmailAddress::encodeMailtoUrl(blockAddrs.first());
                            signer = QLatin1String("<a href=\"mailto:") + QLatin1String(QUrl::toPercentEncoding(address.path())) +
                                     QLatin1String("\">") + signer + QLatin1String("</a>");
                        }
                    }

                    if (block.keyId.isEmpty()) {
                        if (signer.isEmpty() || onlyShowKeyURL) {
                            htmlStr += i18n("Message was signed with unknown key.");
                        } else
                            htmlStr += i18n("Message was signed by %1.",
                                            signer);
                    } else {
                        QDateTime created = block.creationTime;
                        if (created.isValid()) {
                            if (signer.isEmpty()) {
                                if (onlyShowKeyURL)
                                    htmlStr += i18n("Message was signed with key %1.",
                                                    keyWithWithoutURL);
                                else
                                    htmlStr += i18n("Message was signed on %1 with key %2.",
                                                    QLocale::system().toString(created, QLocale::ShortFormat),
                                                    keyWithWithoutURL);
                            } else {
                                if (onlyShowKeyURL)
                                    htmlStr += i18n("Message was signed with key %1.",
                                                    keyWithWithoutURL);
                                else
                                    htmlStr += i18n("Message was signed by %3 on %1 with key %2",
                                                    QLocale::system().toString(created, QLocale::ShortFormat),
                                                    keyWithWithoutURL,
                                                    signer);
                            }
                        } else {
                            if (signer.isEmpty() || onlyShowKeyURL)
                                htmlStr += i18n("Message was signed with key %1.",
                                                keyWithWithoutURL);
                            else
                                htmlStr += i18n("Message was signed by %2 with key %1.",
                                                keyWithWithoutURL,
                                                signer);
                        }
                    }
                }
                htmlStr += QLatin1String("<br />");
                if (!statusStr.isEmpty()) {
                    htmlStr += QLatin1String("&nbsp;<br />");
                    htmlStr += i18n("Status: ");
                    htmlStr += statusStr;
                }
            } else {
                htmlStr += statusStr;
            }
            frame = QLatin1String("</td></tr><tr class=\"") + block.signClass + QLatin1String("B\"><td>");
            htmlStr += endVerboseSigstatHeader(block) + frame;
            simpleHtmlStr += frame;

        } else {

            // old frame settings for PGP:

            if (block.signer.isEmpty() || block.technicalProblem) {
                block.signClass = QStringLiteral("signWarn");
                QString frame = QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" "
                                "class=\"") + block.signClass + QLatin1String("\">"
                                        "<tr class=\"") + block.signClass + QLatin1String("H\"><td dir=\"") + dir + QLatin1String("\">");
                htmlStr += frame + beginVerboseSigstatHeader();
                simpleHtmlStr += frame;
                simpleHtmlStr += writeSimpleSigstatHeader(block, mPrinting);
                if (block.technicalProblem) {
                    htmlStr += block.errorText;
                } else {
                    if (!block.keyId.isEmpty()) {
                        QDateTime created = block.creationTime;
                        if (created.isValid())
                            htmlStr += i18n("Message was signed on %1 with unknown key %2.",
                                            QLocale::system().toString(created, QLocale::ShortFormat),
                                            keyWithWithoutURL);
                        else
                            htmlStr += i18n("Message was signed with unknown key %1.",
                                            keyWithWithoutURL);
                    } else {
                        htmlStr += i18n("Message was signed with unknown key.");
                    }
                    htmlStr += QLatin1String("<br />");
                    htmlStr += i18n("The validity of the signature cannot be "
                                    "verified.");
                    if (!statusStr.isEmpty()) {
                        htmlStr += QLatin1String("<br />");
                        htmlStr += i18n("Status: ");
                        htmlStr += QLatin1String("<i>");
                        htmlStr += statusStr;
                        htmlStr += QLatin1String("</i>");
                    }
                }
                frame = QLatin1String("</td></tr><tr class=\"") + block.signClass + QLatin1String("B\"><td>");
                htmlStr += endVerboseSigstatHeader(block) + frame;
                simpleHtmlStr += frame;
            } else {
                // HTMLize the signer's user id and create mailto: link
                signer = StringUtil::quoteHtmlChars(signer, true);
                signer = QLatin1String("<a href=\"mailto:") + signer + QLatin1String("\">") + signer + QLatin1String("</a>");

                if (block.isGoodSignature) {
                    if (block.keyTrust < GpgME::Signature::Marginal) {
                        block.signClass = QStringLiteral("signOkKeyBad");
                    } else {
                        block.signClass = QStringLiteral("signOkKeyOk");
                    }
                    QString frame = QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" "
                                    "class=\"") + block.signClass + QLatin1String("\">"
                                            "<tr class=\"") + block.signClass + QLatin1String("H\"><td dir=\"") + dir + QLatin1String("\">");
                    htmlStr += frame + beginVerboseSigstatHeader();
                    simpleHtmlStr += frame;
                    simpleHtmlStr += writeSimpleSigstatHeader(block, mPrinting);
                    if (!block.keyId.isEmpty())
                        htmlStr += i18n("Message was signed by %2 (Key ID: %1).",
                                        keyWithWithoutURL,
                                        signer);
                    else {
                        htmlStr += i18n("Message was signed by %1.", signer);
                    }
                    htmlStr += QLatin1String("<br />");

                    switch (block.keyTrust) {
                    case GpgME::Signature::Unknown:
                        htmlStr += i18n("The signature is valid, but the key's "
                                        "validity is unknown.");
                        break;
                    case GpgME::Signature::Marginal:
                        htmlStr += i18n("The signature is valid and the key is "
                                        "marginally trusted.");
                        break;
                    case GpgME::Signature::Full:
                        htmlStr += i18n("The signature is valid and the key is "
                                        "fully trusted.");
                        break;
                    case GpgME::Signature::Ultimate:
                        htmlStr += i18n("The signature is valid and the key is "
                                        "ultimately trusted.");
                        break;
                    default:
                        htmlStr += i18n("The signature is valid, but the key is "
                                        "untrusted.");
                    }
                    frame = QLatin1String("</td></tr>"
                                          "<tr class=\"") + block.signClass + QLatin1String("B\"><td>");
                    htmlStr += endVerboseSigstatHeader(block) + frame;
                    simpleHtmlStr += frame;
                } else {
                    block.signClass = QStringLiteral("signErr");
                    QString frame = QLatin1String("<table cellspacing=\"1\" ") + cellPadding + QLatin1String(" "
                                    "class=\"") + block.signClass + QLatin1String("\">"
                                            "<tr class=\"") + block.signClass + QLatin1String("H\"><td dir=\"") + dir + QLatin1String("\">");
                    htmlStr += frame + beginVerboseSigstatHeader();
                    simpleHtmlStr += frame;
                    simpleHtmlStr += writeSimpleSigstatHeader(block, mPrinting);
                    if (!block.keyId.isEmpty())
                        htmlStr += i18n("Message was signed by %2 (Key ID: %1).",
                                        keyWithWithoutURL,
                                        signer);
                    else {
                        htmlStr += i18n("Message was signed by %1.", signer);
                    }
                    htmlStr += QLatin1String("<br />");
                    htmlStr += i18n("Warning: The signature is bad.");
                    frame = QLatin1String("</td></tr>"
                                          "<tr class=\"") + block.signClass + QLatin1String("B\"><td>");
                    htmlStr += endVerboseSigstatHeader(block) + frame;
                    simpleHtmlStr += frame;
                }
            }
        }
    }

    if (mSource->showSignatureDetails()) {
        return htmlStr;
    }
    return simpleHtmlStr;
}

QString ObjectTreeParser::writeSigstatFooter(PartMetaData &block)
{
    const QString dir = (QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr"));

    QString htmlStr;

    if (block.isSigned) {
        htmlStr += QLatin1String("</td></tr><tr class=\"") + block.signClass + QLatin1String("H\">");
        htmlStr += QLatin1String("<td dir=\"") + dir + QLatin1String("\">") +
                   i18n("End of signed message") +
                   QLatin1String("</td></tr></table>");
    }

    if (block.isEncrypted) {
        htmlStr += QLatin1String("</td></tr><tr class=\"encrH\"><td dir=\"") + dir + QLatin1String("\">") +
                   i18n("End of encrypted message") +
                   QLatin1String("</td></tr></table>");
    }

    if (block.isEncapsulatedRfc822Message) {
        htmlStr += QLatin1String("</td></tr><tr class=\"rfc822H\"><td dir=\"") + dir + QLatin1String("\">") +
                   i18n("End of encapsulated message") +
                   QLatin1String("</td></tr></table>");
    }

    return htmlStr;
}

//-----------------------------------------------------------------------------

bool ObjectTreeParser::okVerify(const QByteArray &data, const Kleo::CryptoBackend::Protocol *cryptProto, PartMetaData &messagePart, QByteArray &verifiedText, std::vector <GpgME::Signature> &signatures, const QByteArray &signature, KMime::Content *sign)
{
    enum { NO_PLUGIN, NOT_INITIALIZED, CANT_VERIFY_SIGNATURES }
    cryptPlugError = NO_PLUGIN;

    QString cryptPlugLibName;
    QString cryptPlugDisplayName;
    if (cryptProto) {
        cryptPlugLibName = cryptProto->name();
        cryptPlugDisplayName = cryptProto->displayName();
    }

    messagePart.isSigned = false;
    messagePart.technicalProblem = (cryptProto == 0);
    messagePart.keyTrust = GpgME::Signature::Unknown;
    messagePart.status = i18n("Wrong Crypto Plug-In.");
    messagePart.status_code = GPGME_SIG_STAT_NONE;

    const QByteArray mementoName = "verification";

    CryptoBodyPartMemento *m = dynamic_cast<CryptoBodyPartMemento *>(mNodeHelper->bodyPartMemento(sign, mementoName));

    if (!m) {
        if (!signature.isEmpty()) {
            Kleo::VerifyDetachedJob *job = cryptProto->verifyDetachedJob();
            if (job) {
                m = new VerifyDetachedBodyPartMemento(job, cryptProto->keyListJob(), signature, data);
            } else {
                cryptPlugError = CANT_VERIFY_SIGNATURES;
                cryptProto = 0;
            }
        } else {
            Kleo::VerifyOpaqueJob *job = cryptProto->verifyOpaqueJob();
            if (job) {
                m = new VerifyOpaqueBodyPartMemento(job, cryptProto->keyListJob(), data);
            } else {
                cryptPlugError = CANT_VERIFY_SIGNATURES;
                cryptProto = 0;
            }
        }
        if (m) {
            if (allowAsync()) {
                QObject::connect(m, &CryptoBodyPartMemento::update,
                                 mNodeHelper, &NodeHelper::update);
                QObject::connect(m, SIGNAL(update(MessageViewer::Viewer::UpdateMode)),
                                 mSource->sourceObject(), SLOT(update(MessageViewer::Viewer::UpdateMode)));

                if (m->start()) {
                    messagePart.inProgress = true;
                    mHasPendingAsyncJobs = true;
                }
            } else {
                m->exec();
            }
            mNodeHelper->setBodyPartMemento(sign, mementoName, m);
        }
    } else if (m->isRunning()) {
        messagePart.inProgress = true;
        mHasPendingAsyncJobs = true;
        m = 0;
    } else {
        messagePart.inProgress = false;
        mHasPendingAsyncJobs = false;
    }

    if (m && !messagePart.inProgress) {
        if (!signature.isEmpty()) {
            VerifyDetachedBodyPartMemento *vm = dynamic_cast<VerifyDetachedBodyPartMemento *>(m);
            verifiedText = data;
            signatures = vm->verifyResult().signatures();
        } else {
            VerifyOpaqueBodyPartMemento *vm = dynamic_cast<VerifyOpaqueBodyPartMemento *>(m);
            verifiedText = vm->plainText();
            signatures = vm->verifyResult().signatures();
        }
        messagePart.auditLogError = m->auditLogError();
        messagePart.auditLog = m->auditLogAsHtml();
        messagePart.isSigned = !signatures.empty();
    }

    if (!cryptProto) {
        QString errorMsg;
        switch (cryptPlugError) {
        case NOT_INITIALIZED:
            errorMsg = i18n("Crypto plug-in \"%1\" is not initialized.",
                            cryptPlugLibName);
            break;
        case CANT_VERIFY_SIGNATURES:
            errorMsg = i18n("Crypto plug-in \"%1\" cannot verify signatures.",
                            cryptPlugLibName);
            break;
        case NO_PLUGIN:
            if (cryptPlugDisplayName.isEmpty()) {
                errorMsg = i18n("No appropriate crypto plug-in was found.");
            } else {
                errorMsg = i18nc("%1 is either 'OpenPGP' or 'S/MIME'",
                                 "No %1 plug-in was found.",
                                 cryptPlugDisplayName);
            }
            break;
        }
        messagePart.errorText = i18n("The message is signed, but the "
                                     "validity of the signature cannot be "
                                     "verified.<br />"
                                     "Reason: %1",
                                     errorMsg);
    }

    return messagePart.isSigned;
}

void ObjectTreeParser::sigStatusToMetaData(const std::vector <GpgME::Signature> &signatures, const Kleo::CryptoBackend::Protocol *cryptProto, PartMetaData &messagePart, GpgME::Key key)
{
    if (messagePart.isSigned) {
        GpgME::Signature signature = signatures.front();
        messagePart.status_code = signatureToStatus(signature);
        messagePart.isGoodSignature = messagePart.status_code & GPGME_SIG_STAT_GOOD;
        // save extended signature status flags
        messagePart.sigSummary = signature.summary();

        if (messagePart.isGoodSignature && !key.keyID()) {
            // Search for the key by it's fingerprint so that we can check for
            // trust etc.
            Kleo::KeyListJob *job = cryptProto->keyListJob(false);    // local, no sigs
            if (!job) {
                qCDebug(MESSAGEVIEWER_LOG) << "The Crypto backend does not support listing keys. ";
            } else {
                std::vector<GpgME::Key> found_keys;
                // As we are local it is ok to make this synchronous
                GpgME::KeyListResult res = job->exec(QStringList(QLatin1String(signature.fingerprint())), false, found_keys);
                if (res.error()) {
                    qCDebug(MESSAGEVIEWER_LOG) << "Error while searching key for Fingerprint: " << signature.fingerprint();
                }
                if (found_keys.size() > 1) {
                    // Should not Happen
                    qCDebug(MESSAGEVIEWER_LOG) << "Oops: Found more then one Key for Fingerprint: " << signature.fingerprint();
                }
                if (found_keys.size() != 1) {
                    // Should not Happen at this point
                    qCDebug(MESSAGEVIEWER_LOG) << "Oops: Found no Key for Fingerprint: " << signature.fingerprint();
                } else {
                    key = found_keys[0];
                }
            }
        }

        if (key.keyID()) {
            messagePart.keyId = key.keyID();
        }
        if (messagePart.keyId.isEmpty()) {
            messagePart.keyId = signature.fingerprint();
        }
        messagePart.keyTrust = signature.validity();
        if (key.numUserIDs() > 0 && key.userID(0).id()) {
            messagePart.signer = Kleo::DN(key.userID(0).id()).prettyDN();
        }
        for (uint iMail = 0; iMail < key.numUserIDs(); ++iMail) {
            // The following if /should/ always result in TRUE but we
            // won't trust implicitely the plugin that gave us these data.
            if (key.userID(iMail).email()) {
                QString email = QString::fromUtf8(key.userID(iMail).email());
                // ### work around gpgme 0.3.x / cryptplug bug where the
                // ### email addresses are specified as angle-addr, not addr-spec:
                if (email.startsWith(QLatin1Char('<')) && email.endsWith(QLatin1Char('>'))) {
                    email = email.mid(1, email.length() - 2);
                }
                if (!email.isEmpty()) {
                    messagePart.signerMailAddresses.append(email);
                }
            }
        }

        if (signature.creationTime()) {
            messagePart.creationTime.setTime_t(signature.creationTime());
        } else {
            messagePart.creationTime = QDateTime();
        }
        if (messagePart.signer.isEmpty()) {
            if (key.numUserIDs() > 0 && key.userID(0).name()) {
                messagePart.signer = Kleo::DN(key.userID(0).name()).prettyDN();
            }
            if (!messagePart.signerMailAddresses.empty()) {
                if (messagePart.signer.isEmpty()) {
                    messagePart.signer = messagePart.signerMailAddresses.front();
                } else {
                    messagePart.signer += QLatin1String(" <") + messagePart.signerMailAddresses.front() + QLatin1Char('>');
                }
            }
        }
    }
}

static QString iconToDataUrl(const QString &iconPath)
{
    QFile f(iconPath);
    if (!f.open(QIODevice::ReadOnly)) {
        return QString();
    }

    const QByteArray ba = f.readAll();
    return QStringLiteral("data:image/png;base64,%1").arg(QLatin1String(ba.toBase64().constData()));
}

QString ObjectTreeParser::quotedHTML(const QString &s, bool decorate)
{
    assert(cssHelper());

    KTextToHTML::Options convertFlags = KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText;
    if (decorate && MessageViewer::MessageViewerSettings::self()->showEmoticons()) {
        convertFlags |= KTextToHTML::ReplaceSmileys;
    }
    QString htmlStr;
    const QString normalStartTag = cssHelper()->nonQuotedFontTag();
    QString quoteFontTag[3];
    QString deepQuoteFontTag[3];
    for (int i = 0; i < 3; ++i) {
        quoteFontTag[i] = cssHelper()->quoteFontTag(i);
        deepQuoteFontTag[i] = cssHelper()->quoteFontTag(i + 3);
    }
    const QString normalEndTag = QStringLiteral("</div>");
    const QString quoteEnd = QStringLiteral("</div>");

    const unsigned int length = s.length();
    bool paraIsRTL = false;
    bool startNewPara = true;
    unsigned int pos, beg;

    // skip leading empty lines
    for (pos = 0; pos < length && s[pos] <= QLatin1Char(' '); ++pos)
        ;
    while (pos > 0 && (s[pos - 1] == QLatin1Char(' ') || s[pos - 1] == QLatin1Char('\t'))) {
        pos--;
    }
    beg = pos;

    int currQuoteLevel = -2; // -2 == no previous lines
    bool curHidden = false; // no hide any block

    if (MessageViewer::MessageViewerSettings::self()->showExpandQuotesMark()) {
        // Cache Icons
        if (mCollapseIcon.isEmpty()) {
            mCollapseIcon = iconToDataUrl(IconNameCache::instance()->iconPath(QStringLiteral("quotecollapse"), 0));
        }
        if (mExpandIcon.isEmpty()) {
            mExpandIcon = iconToDataUrl(IconNameCache::instance()->iconPath(QStringLiteral("quoteexpand"), 0));
        }
    }

    while (beg < length) {
        /* search next occurrence of '\n' */
        pos = s.indexOf(QLatin1Char('\n'), beg, Qt::CaseInsensitive);
        if (pos == (unsigned int)(-1)) {
            pos = length;
        }

        QString line(s.mid(beg, pos - beg));
        beg = pos + 1;

        /* calculate line's current quoting depth */
        int actQuoteLevel = -1;
        const int numberOfCaracters(line.length());
        for (int p = 0; p < numberOfCaracters; ++p) {
            switch (line[p].toLatin1()) {
            case '>':
            case '|':
                actQuoteLevel++;
                break;
            case ' ':  // spaces and tabs are allowed between the quote markers
            case '\t':
            case '\r':
                break;
            default:  // stop quoting depth calculation
                p = numberOfCaracters;
                break;
            }
        } /* for() */

        bool actHidden = false;

        // This quoted line needs be hidden
        if (MessageViewer::MessageViewerSettings::self()->showExpandQuotesMark() && mSource->levelQuote() >= 0
                && mSource->levelQuote() <= (actQuoteLevel)) {
            actHidden = true;
        }

        if (actQuoteLevel != currQuoteLevel) {
            /* finish last quotelevel */
            if (currQuoteLevel == -1) {
                htmlStr.append(normalEndTag);
            } else if (currQuoteLevel >= 0 && !curHidden) {
                htmlStr.append(quoteEnd);
            }

            /* start new quotelevel */
            if (actQuoteLevel == -1) {
                htmlStr += normalStartTag;
            } else {
                if (MessageViewer::MessageViewerSettings::self()->showExpandQuotesMark()) {
                    if (actHidden) {
                        //only show the QuoteMark when is the first line of the level hidden
                        if (!curHidden) {
                            //Expand all quotes
                            htmlStr += QLatin1String("<div class=\"quotelevelmark\" >");
                            htmlStr += QStringLiteral("<a href=\"kmail:levelquote?%1 \">"
                                                      "<img src=\"%2\" alt=\"\" title=\"\"/></a>")
                                       .arg(-1)
                                       .arg(mExpandIcon);
                            htmlStr += QLatin1String("</div><br/>");
                            htmlStr += quoteEnd;
                        }
                    } else {
                        htmlStr += QLatin1String("<div class=\"quotelevelmark\" >");
                        htmlStr += QStringLiteral("<a href=\"kmail:levelquote?%1 \">"
                                                  "<img src=\"%2\" alt=\"\" title=\"\"/></a>")
                                   .arg(actQuoteLevel)
                                   .arg(mCollapseIcon);
                        htmlStr += QLatin1String("</div>");
                        if (actQuoteLevel < 3) {
                            htmlStr += quoteFontTag[actQuoteLevel];
                        } else {
                            htmlStr += deepQuoteFontTag[actQuoteLevel % 3];
                        }
                    }
                } else {
                    if (actQuoteLevel < 3) {
                        htmlStr += quoteFontTag[actQuoteLevel];
                    } else {
                        htmlStr += deepQuoteFontTag[actQuoteLevel % 3];
                    }
                }
            }
            currQuoteLevel = actQuoteLevel;
        }
        curHidden = actHidden;

        if (!actHidden) {
            // don't write empty <div ...></div> blocks (they have zero height)
            // ignore ^M DOS linebreaks
            if (!line.remove(QLatin1Char('\015')).isEmpty()) {
                if (startNewPara) {
                    paraIsRTL = line.isRightToLeft();
                }
                htmlStr += QStringLiteral("<div dir=\"%1\">").arg(paraIsRTL ? QStringLiteral("rtl") : QStringLiteral("ltr"));
                htmlStr += KTextToHTML::convertToHtml(line, convertFlags);
                htmlStr += QLatin1String("</div>");
                startNewPara = looksLikeParaBreak(s, pos);
            } else {
                htmlStr += QLatin1String("<br/>");
                // after an empty line, always start a new paragraph
                startNewPara = true;
            }
        }
    } /* while() */

    /* really finish the last quotelevel */
    if (currQuoteLevel == -1) {
        htmlStr.append(normalEndTag);
    } else {
        htmlStr.append(quoteEnd);
    }

    qCDebug(MESSAGEVIEWER_LOG) << "========================================\n"
                               << htmlStr
                               << "\n======================================\n";
    return htmlStr;
}

const QTextCodec *ObjectTreeParser::codecFor(KMime::Content *node) const
{
    assert(node);
    if (mSource->overrideCodec()) {
        return mSource->overrideCodec();
    }
    return mNodeHelper->codec(node);
}

// Guesstimate if the newline at newLinePos actually separates paragraphs in the text s
// We use several heuristics:
// 1. If newLinePos points after or before (=at the very beginning of) text, it is not between paragraphs
// 2. If the previous line was longer than the wrap size, we want to consider it a paragraph on its own
//    (some clients, notably Outlook, send each para as a line in the plain-text version).
// 3. Otherwise, we check if the newline could have been inserted for wrapping around; if this
//    was the case, then the previous line will be shorter than the wrap size (which we already
//    know because of item 2 above), but adding the first word from the next line will make it
//    longer than the wrap size.
bool ObjectTreeParser::looksLikeParaBreak(const QString &s, unsigned int newLinePos) const
{
    const unsigned int WRAP_COL = 78;

    unsigned int length = s.length();
    // 1. Is newLinePos at an end of the text?
    if (newLinePos >= length - 1 || newLinePos == 0) {
        return false;
    }

    // 2. Is the previous line really a paragraph -- longer than the wrap size?

    // First char of prev line -- works also for first line
    unsigned prevStart = s.lastIndexOf(QLatin1Char('\n'), newLinePos - 1) + 1;
    unsigned prevLineLength = newLinePos - prevStart;
    if (prevLineLength > WRAP_COL) {
        return true;
    }

    // find next line to delimit search for first word
    unsigned int nextStart = newLinePos + 1;
    int nextEnd = s.indexOf(QLatin1Char('\n'), nextStart);
    if (nextEnd == -1) {
        nextEnd = length;
    }
    QString nextLine = s.mid(nextStart, nextEnd - nextStart);
    length = nextLine.length();
    // search for first word in next line
    unsigned int wordStart;
    bool found = false;
    for (wordStart = 0; !found && wordStart < length; wordStart++) {
        switch (nextLine[wordStart].toLatin1()) {
        case '>':
        case '|':
        case ' ':  // spaces, tabs and quote markers don't count
        case '\t':
        case '\r':
            break;
        default:
            found = true;
            break;
        }
    } /* for() */

    if (!found) {
        // next line is essentially empty, it seems -- empty lines are
        // para separators
        return true;
    }
    //Find end of first word.
    //Note: flowText (in kmmessage.cpp) separates words for wrap by
    //spaces only. This should be consistent, which calls for some
    //refactoring.
    int wordEnd = nextLine.indexOf(QLatin1Char(' '), wordStart);
    if (wordEnd == (-1)) {
        wordEnd = length;
    }
    int wordLength = wordEnd - wordStart;

    // 3. If adding a space and the first word to the prev line don't
    //    make it reach the wrap column, then the break was probably
    //    meaningful
    return prevLineLength + wordLength + 1 < WRAP_COL;
}

#ifdef MARCS_DEBUG
void ObjectTreeParser::dumpToFile(const char *filename, const char *start,
                                  size_t len)
{
    assert(filename);

    QFile f(QString::fromAscii(filename));
    if (f.open(QIODevice::WriteOnly)) {
        if (start) {
            QDataStream ds(&f);
            ds.writeRawData(start, len);
        }
        f.close();  // If data is 0 we just create a zero length file.
    }
}
#endif // !NDEBUG

KMime::Content *ObjectTreeParser::findType(KMime::Content *content, const QByteArray &mimeType, bool deep, bool wide)
{
    if ((!content->contentType()->isEmpty())
            && (mimeType.isEmpty()  || (mimeType == content->contentType()->mimeType()))) {
        return content;
    }
    KMime::Content *child = MessageCore::NodeHelper::firstChild(content);
    if (child && deep) { //first child
        return findType(child, mimeType, deep, wide);
    }

    KMime::Content *next = MessageCore::NodeHelper::nextSibling(content);
    if (next &&  wide) { //next on the same level
        return findType(next, mimeType, deep, wide);
    }

    return 0;
}

KMime::Content *ObjectTreeParser::findType(KMime::Content *content, const QByteArray &mediaType, const QByteArray &subType, bool deep, bool wide)
{
    if (!content->contentType()->isEmpty()) {
        if ((mediaType.isEmpty()  ||  mediaType == content->contentType()->mediaType())
                && (subType.isEmpty()  ||  subType == content->contentType()->subType())) {
            return content;
        }
    }
    KMime::Content *child = MessageCore::NodeHelper::firstChild(content);
    if (child && deep) { //first child
        return findType(child, mediaType, subType, deep, wide);
    }

    KMime::Content *next = MessageCore::NodeHelper::nextSibling(content);
    if (next &&  wide) { //next on the same level
        return findType(next, mediaType, subType, deep, wide);
    }

    return 0;
}

KMime::Content *ObjectTreeParser::findTypeNot(KMime::Content *content, const QByteArray &mediaType, const QByteArray &subType, bool deep, bool wide)
{
    if ((!content->contentType()->isEmpty())
            && (mediaType.isEmpty() || content->contentType()->mediaType() != mediaType)
            && (subType.isEmpty() || content->contentType()->subType() != subType)
       ) {
        return content;
    }
    KMime::Content *child = MessageCore::NodeHelper::firstChild(content);
    if (child && deep) {
        return findTypeNot(child, mediaType, subType, deep, wide);
    }

    KMime::Content *next = MessageCore::NodeHelper::nextSibling(content);
    if (next && wide) {
        return findTypeNot(next,  mediaType, subType, deep, wide);
    }
    return 0;
}

QString ObjectTreeParser::convertedTextContent() const
{
    QString plainTextContent = mPlainTextContent;
    if (plainTextContent.isEmpty()) {
        QWebPage doc;
        doc.mainFrame()->setHtml(mHtmlContent);
        plainTextContent = doc.mainFrame()->toPlainText();
    }
    return plainTextContent.append(QLatin1Char('\n'));
}

QString ObjectTreeParser::convertedHtmlContent() const
{
    QString htmlContent = mHtmlContent;
    if (htmlContent.isEmpty()) {
        QString convertedHtml = mPlainTextContent.toHtmlEscaped();
        convertedHtml.append(QStringLiteral("</body></html>"));
        convertedHtml.prepend(QStringLiteral("<html><head></head><body>"));
        htmlContent = convertedHtml.replace(QStringLiteral("\n"), QStringLiteral("<br />"));
    }
    return htmlContent.append(QLatin1Char('\n'));
}

QByteArray ObjectTreeParser::plainTextContentCharset() const
{
    return mPlainTextContentCharset;
}

QByteArray ObjectTreeParser::htmlContentCharset() const
{
    return mHtmlContentCharset;
}

void ObjectTreeParser::setCryptoProtocol(const Kleo::CryptoBackend::Protocol *protocol)
{
    mCryptoProtocol = protocol;
}

const Kleo::CryptoBackend::Protocol *ObjectTreeParser::cryptoProtocol() const
{
    return mCryptoProtocol;
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

HtmlWriter *ObjectTreeParser::htmlWriter() const
{
    if (mHtmlWriter) {
        return mHtmlWriter;
    }
    return mSource->htmlWriter();
}

CSSHelper *ObjectTreeParser::cssHelper() const
{
    return mSource->cssHelper();
}

MessageViewer::NodeHelper *ObjectTreeParser::nodeHelper() const
{
    return mNodeHelper;
}
