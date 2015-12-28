/*
   Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "messagepart.h"
#include "messageviewer_debug.h"
#include <libkleo/importjob.h>
#include "objecttreeparser.h"
#include "converthtmltoplaintext.h"
#include "htmlquotecolorer.h"
#include "csshelper.h"
#include "cryptohelper.h"

#include <MessageCore/StringUtil>

#include <libkleo/importjob.h>
#include <libkleo/cryptobackendfactory.h>

#include <interfaces/htmlwriter.h>
#include <htmlwriter/queuehtmlwriter.h>
#include <job/kleojobexecutor.h>
#include <settings/messageviewersettings.h>
#include <kmime/kmime_content.h>
#include <gpgme++/key.h>
#include <gpgme.h>

#include <QTextCodec>
#include <QApplication>

#include <KLocalizedString>

using namespace MessageViewer;

/** Checks whether @p str contains external references. To be precise,
    we only check whether @p str contains 'xxx="http[s]:' where xxx is
    not href. Obfuscated external references are ignored on purpose.
*/

bool containsExternalReferences(const QString &str, const QString &extraHead)
{
    const bool hasBaseInHeader = extraHead.contains(QStringLiteral("<base href=\""), Qt::CaseInsensitive);
    if (hasBaseInHeader && (str.contains(QStringLiteral("href=\"/"), Qt::CaseInsensitive) ||
                            str.contains(QStringLiteral("<img src=\"/"), Qt::CaseInsensitive))) {
        return true;
    }
    /*
    //Laurent: workaround for local ref cid
    if(str.contains(QStringLiteral("<img src=\"cid:"),Qt::CaseInsensitive)) {
    return true;
    }
    */
    int httpPos = str.indexOf(QLatin1String("\"http:"), Qt::CaseInsensitive);
    int httpsPos = str.indexOf(QLatin1String("\"https:"), Qt::CaseInsensitive);

    while (httpPos >= 0 || httpsPos >= 0) {
        // pos = index of next occurrence of "http: or "https: whichever comes first
        int pos = (httpPos < httpsPos)
                  ? ((httpPos >= 0) ? httpPos : httpsPos)
                  : ((httpsPos >= 0) ? httpsPos : httpPos);
        // look backwards for "href"
        if (pos > 5) {
            int hrefPos = str.lastIndexOf(QLatin1String("href"), pos - 5, Qt::CaseInsensitive);
            // if no 'href' is found or the distance between 'href' and '"http[s]:'
            // is larger than 7 (7 is the distance in 'href = "http[s]:') then
            // we assume that we have found an external reference
            if ((hrefPos == -1) || (pos - hrefPos > 7)) {

                // HTML messages created by KMail itself for now contain the following:
                // <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
                // Make sure not to show an external references warning for this string
                int dtdPos = str.indexOf(QLatin1String("http://www.w3.org/TR/html4/loose.dtd"), pos + 1);
                if (dtdPos != (pos + 1)) {
                    return true;
                }
            }
        }
        // find next occurrence of "http: or "https:
        if (pos == httpPos) {
            httpPos = str.indexOf(QLatin1String("\"http:"), httpPos + 6, Qt::CaseInsensitive);
        } else {
            httpsPos = str.indexOf(QLatin1String("\"https:"), httpsPos + 7, Qt::CaseInsensitive);
        }
    }
    return false;
}

//--------CryptoBlock-------------------
CryptoBlock::CryptoBlock(ObjectTreeParser *otp,
                         PartMetaData *block,
                         const Kleo::CryptoBackend::Protocol *cryptoProto,
                         const QString &fromAddress,
                         KMime::Content *node)
    : HTMLBlock()
    , mOtp(otp)
    , mMetaData(block)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mNode(node)
{
    internalEnter();
}

CryptoBlock::~CryptoBlock()
{
    internalExit();
}

void CryptoBlock::internalEnter()
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (writer && !entered) {
        entered = true;
        writer->queue(mOtp->writeSigstatHeader(*mMetaData, mCryptoProto, mFromAddress, mNode));
    }
}

void CryptoBlock::internalExit()
{
    if (!entered) {
        return;
    }
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    writer->queue(mOtp->writeSigstatFooter(*mMetaData));
    entered = false;
}

AttachmentMarkBlock::AttachmentMarkBlock(MessageViewer::HtmlWriter *writer, KMime::Content *node)
    : mNode(node)
    , mWriter(writer)
{
    internalEnter();
}

AttachmentMarkBlock::~AttachmentMarkBlock()
{
    internalExit();
}

void AttachmentMarkBlock::internalEnter()
{
    if (mWriter && !entered) {
        const QString index = mNode->index().toString();
        mWriter->queue(QStringLiteral("<a name=\"att%1\"></a>").arg(index));
        mWriter->queue(QStringLiteral("<div id=\"attachmentDiv%1\">\n").arg(index));
        entered = true;
    }
}

void AttachmentMarkBlock::internalExit()
{
    if (!entered) {
        return;
    }

    mWriter->queue(QStringLiteral("</div>"));
    entered = false;
}

TextBlock::TextBlock(MessageViewer::HtmlWriter *writer, MessageViewer::NodeHelper *nodeHelper, KMime::Content *node, bool link)
    : mWriter(writer)
    , mNodeHelper(nodeHelper)
    , mNode(node)
    , mLink(link)
{
    internalEnter();
}

TextBlock::~TextBlock()
{
    internalExit();
}

void TextBlock::internalEnter()
{
    if (!mWriter || entered) {
        return;
    }
    entered = true;

    const QString label = MessageCore::StringUtil::quoteHtmlChars(NodeHelper::fileName(mNode), true);

    const QString comment =
        MessageCore::StringUtil::quoteHtmlChars(mNode->contentDescription()->asUnicodeString(), true);

    const QString dir = QApplication::isRightToLeft() ? QStringLiteral("rtl") : QStringLiteral("ltr");

    mWriter->queue(QLatin1String("<table cellspacing=\"1\" class=\"textAtm\">"
                                 "<tr class=\"textAtmH\"><td dir=\"") + dir + QLatin1String("\">"));
    if (!mLink)
        mWriter->queue(QLatin1String("<a href=\"") + mNodeHelper->asHREF(mNode, QStringLiteral("body")) + QLatin1String("\">")
                       + label + QLatin1String("</a>"));
    else {
        mWriter->queue(label);
    }
    if (!comment.isEmpty()) {
        mWriter->queue(QLatin1String("<br/>") + comment);
    }
    mWriter->queue(QLatin1String("</td></tr><tr class=\"textAtmB\"><td>"));
}

void TextBlock::internalExit()
{
    if (!entered) {
        return;
    }

    entered = false;

    mWriter->queue(QStringLiteral("</td></tr></table>"));
}

HTMLWarnBlock::HTMLWarnBlock(HtmlWriter* writer, const QString& msg)
    : mWriter(writer)
    , mMsg(msg)
{
    internalEnter();
}

HTMLWarnBlock::~HTMLWarnBlock()
{
    internalExit();
}

void HTMLWarnBlock::internalEnter()
{
    if (!mWriter || entered) {
        return;
    }
    entered = true;

    if (!mMsg.isEmpty()) {
        mWriter->queue(QStringLiteral("<div class=\"htmlWarn\">\n"));
        mWriter->queue(mMsg);
        mWriter->queue(QStringLiteral("</div><br/><br/>"));
    }

    mWriter->queue(QStringLiteral("<div style=\"position: relative\">\n"));
}

void HTMLWarnBlock::internalExit()
{
    if (!entered) {
        return;
    }

    entered = false;

    mWriter->queue(QStringLiteral("</div>\n"));
}

//------MessagePart-----------------------
MessagePart::MessagePart(ObjectTreeParser *otp,
                         const QString &text)
    : mText(text)
    , mOtp(otp)
    , mSubOtp(Q_NULLPTR)
{

}

MessagePart::~MessagePart()
{
    if (mSubOtp) {
        delete mSubOtp->htmlWriter();
        delete mSubOtp;
        mSubOtp = Q_NULLPTR;
    }
}

PartMetaData *MessagePart::partMetaData()
{
    return &mMetaData;
}

QString MessagePart::text() const
{
    return mText;
}

void MessagePart::setText(const QString &text)
{
    mText = text;
}

void MessagePart::html(bool decorate)
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, &mMetaData, Q_NULLPTR, QString(), Q_NULLPTR);
    writer->queue(mOtp->quotedHTML(text(), decorate));
}

void MessagePart::parseInternal(KMime::Content *node, bool onlyOneMimePart)
{
    mSubOtp = new ObjectTreeParser(mOtp, onlyOneMimePart);
    mSubOtp->setAllowAsync(mOtp->allowAsync());
    if (mOtp->htmlWriter()) {
        mSubOtp->mHtmlWriter = new QueueHtmlWriter(mOtp->htmlWriter());
    }
    mSubOtp->parseObjectTreeInternal(node);
}

void MessagePart::renderInternalHtml() const
{
    if (mSubOtp && mOtp->htmlWriter()) {
        static_cast<QueueHtmlWriter *>(mSubOtp->htmlWriter())->replay();
    }
}

void MessagePart::copyContentFrom() const
{
    if (mSubOtp) {
        mOtp->copyContentFrom(mSubOtp);
    }
}

QString MessagePart::renderInternalText() const
{
    if (!mSubOtp) {
        return QString();
    }
    return mSubOtp->plainTextContent();
}

//-----TextMessageBlock----------------------

TextMessagePart::TextMessagePart(ObjectTreeParser *otp, KMime::Content *node, bool drawFrame, bool showLink, bool decryptMessage)
    : MessagePart(otp, QString())
    , mNode(node)
    , mDrawFrame(drawFrame)
    , mShowLink(showLink)
    , mDecryptMessage(decryptMessage)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    parseContent();
}

TextMessagePart::~TextMessagePart()
{

}

bool TextMessagePart::decryptMessage() const
{
    return mDecryptMessage;
}

void TextMessagePart::parseContent()
{
    const auto aCodec = mOtp->codecFor(mNode);
    const QString &fromAddress = NodeHelper::fromAsString(mNode);
    mSignatureState  = KMMsgNotSigned;
    mEncryptionState = KMMsgNotEncrypted;
    const auto blocks = prepareMessageForDecryption(mNode->decodedContent());

    const auto cryptProto = Kleo::CryptoBackendFactory::instance()->openpgp();

    if (!blocks.isEmpty()) {

        if (blocks.count() > 1 || blocks.at(0).type() != MessageViewer::NoPgpBlock) {
            mOtp->setCryptoProtocol(cryptProto);
        }


        QString htmlStr;
        QString plainTextStr;

        /* The (overall) signature/encrypted status is broken
         * if one unencrypted part is at the beginning or in the middle
         * because mailmain adds an unencrypted part at the end this should not break the overall status
         *
         * That's why we first set the tmp status and if one crypted/signed block comes afterwards, than
         * the status is set to unencryped
         */
        bool fullySignedOrEncrypted = true;
        bool fullySignedOrEncryptedTmp = true;

        Q_FOREACH (const auto &block, blocks) {

            if (!fullySignedOrEncryptedTmp) {
                fullySignedOrEncrypted = false;
            }

            if (block.type() == NoPgpBlock && !block.text().trimmed().isEmpty()) {
                fullySignedOrEncryptedTmp = false;
                mBlocks.append(MessagePart::Ptr(new MessagePart(mOtp, aCodec->toUnicode(block.text()))));
            } else if (block.type() == PgpMessageBlock) {
                CryptoMessagePart::Ptr mp(new CryptoMessagePart(mOtp, QString(), cryptProto, fromAddress, 0));
                mBlocks.append(mp);
                if (!decryptMessage()) {
                    continue;
                }
                mp->startDecryption(block.text(), aCodec);
                if (mp->partMetaData()->inProgress) {
                    continue;
                }
            } else if (block.type() == ClearsignedBlock) {
                CryptoMessagePart::Ptr mp(new CryptoMessagePart(mOtp, QString(), cryptProto, fromAddress, 0));
                mBlocks.append(mp);
                mp->startVerification(block.text(), aCodec);
            } else {
                continue;
            }

            const PartMetaData *messagePart(mBlocks.last()->partMetaData());

            if (!messagePart->isEncrypted && !messagePart->isSigned && !block.text().trimmed().isEmpty()) {
                mBlocks.last()->setText(aCodec->toUnicode(block.text()));
            }

            if (messagePart->isEncrypted) {
                mEncryptionState = KMMsgPartiallyEncrypted;
            }

            if (messagePart->isSigned) {
                mSignatureState = KMMsgPartiallySigned;
            }
        }

        //Do we have an fully Signed/Encrypted Message?
        if (fullySignedOrEncrypted) {
            if (mSignatureState == KMMsgPartiallySigned) {
                mSignatureState = KMMsgFullySigned;
            }
            if (mEncryptionState == KMMsgPartiallyEncrypted) {
                mEncryptionState = KMMsgFullyEncrypted;
            }
        }
    }
}

void TextMessagePart::html(bool decorate)
{
    HTMLBlock::Ptr block;
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (mDrawFrame) {
        block = HTMLBlock::Ptr(new TextBlock(writer, mOtp->nodeHelper(), mNode, mShowLink));
    }

    foreach (const MessagePart::Ptr &mp, mBlocks) {
        mp->html(decorate);
    }
}

QString TextMessagePart::text() const
{
    QString text;
    foreach (const MessagePart::Ptr &mp, mBlocks) {
        text += mp->text();
    }
    return text;
}

KMMsgEncryptionState TextMessagePart::encryptionState() const
{
    return mEncryptionState;
}

KMMsgSignatureState TextMessagePart::signatureState() const
{
    return mSignatureState;
}

//-----HtmlMessageBlock----------------------

HtmlMessagePart::HtmlMessagePart(ObjectTreeParser* otp, KMime::Content* node, ObjectTreeSourceIf *source)
    : MessagePart(otp, QString())
    , mNode(node)
    , mSource(source)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    const QByteArray partBody(mNode->decodedContent());
    mBodyHTML = mOtp->codecFor(mNode)->toUnicode(partBody);
    mCharset = NodeHelper::charset(mNode);
}

HtmlMessagePart::~HtmlMessagePart()
{
}

void HtmlMessagePart::fix()
{
    mOtp->mHtmlContent += mBodyHTML;
    mOtp->mHtmlContentCharset = mCharset;
}

void HtmlMessagePart::html(bool decorate)
{
    fix();
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (!writer) {
        return;
    }

    HTMLBlock::Ptr block;

    if (mSource->htmlMail()) {
        QString bodyText = mBodyHTML;
        HTMLQuoteColorer colorer;
        colorer.setEnableHtmlQuoteColorer(MessageViewer::MessageViewerSettings::self()->htmlQuoteColorerEnabled());
        QString extraHead;
        for (int i = 0; i < 3; ++i) {
            colorer.setQuoteColor(i, mSource->cssHelper()->quoteColor(i));
        }
        bodyText = colorer.process(bodyText, extraHead);
        mOtp->mNodeHelper->setNodeDisplayedEmbedded(mNode, true);
        writer->extraHead(extraHead);

        // Show the "external references" warning (with possibility to load
        // external references only if loading external references is disabled
        // and the HTML code contains obvious external references). For
        // messages where the external references are obfuscated the user won't
        // have an easy way to load them but that shouldn't be a problem
        // because only spam contains obfuscated external references.
        if (!mSource->htmlLoadExternal() &&
            containsExternalReferences(bodyText, extraHead)) {
            block = HTMLBlock::Ptr(new HTMLWarnBlock(writer, i18n("<b>Note:</b> This HTML message may contain external "
            "references to images etc. For security/privacy reasons "
            "external references are not loaded. If you trust the "
            "sender of this message then you can load the external "
            "references for this message "
            "<a href=\"kmail:loadExternal\">by clicking here</a>.")));
        } else {
            block = HTMLBlock::Ptr(new HTMLWarnBlock(writer, QString()));
        }
        // Make sure the body is relative, so that nothing is painted over above "Note: ..."
        // if a malicious message uses absolute positioning. #137643
        writer->queue(bodyText);
    } else {
        block = HTMLBlock::Ptr(new HTMLWarnBlock(writer, i18n("<b>Note:</b> This is an HTML message. For "
        "security reasons, only the raw HTML code "
        "is shown. If you trust the sender of this "
        "message then you can activate formatted "
        "HTML display for this message "
        "<a href=\"kmail:showHTML\">by clicking here</a>.")));
        // Make sure the body is relative, so that nothing is painted over above "Note: ..."
        // if a malicious message uses absolute positioning. #137643
        ConvertHtmlToPlainText convert;
        convert.setHtmlString(mBodyHTML);
        QString result = convert.generatePlainText();
        result.replace(QLatin1String("\n"), QStringLiteral("<br>"));
        writer->queue(result);
    }
    mSource->setHtmlMode(Util::Html);
}

QString HtmlMessagePart::text() const
{
    return mBodyHTML;
}

//-----MimeMessageBlock----------------------

MimeMessagePart::MimeMessagePart(ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart)
    : MessagePart(otp, QString())
    , mNode(node)
    , mOnlyOneMimePart(onlyOneMimePart)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    parseInternal(mNode, mOnlyOneMimePart);
}

MimeMessagePart::~MimeMessagePart()
{

}

void MimeMessagePart::html(bool decorate)
{
    copyContentFrom();
    renderInternalHtml();
}

QString MimeMessagePart::text() const
{
    return renderInternalText();
}

//-----AlternativeMessagePart----------------------

AlternativeMessagePart::AlternativeMessagePart(ObjectTreeParser* otp, KMime::Content* textNode, KMime::Content* htmlNode)
    : MessagePart(otp, QString())
    , mTextNode(textNode)
    , mHTMLNode(htmlNode)
    , mViewHtml(false)
{
    if (!mTextNode && !mHTMLNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid nodes";
        return;
    }

    if (mTextNode) {
        mTextPart = MimeMessagePart::Ptr(new MimeMessagePart(mOtp, mTextNode, true));
    }

    if (mHTMLNode) {
        mHTMLPart = MimeMessagePart::Ptr(new MimeMessagePart(mOtp, mHTMLNode, true));
    }
}

AlternativeMessagePart::~AlternativeMessagePart()
{

}

void AlternativeMessagePart::setViewHtml(bool html)
{
    mViewHtml = html;
}

bool AlternativeMessagePart::viewHtml()
{
    return mViewHtml;
}

void AlternativeMessagePart::html(bool decorate)
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        // If there is no HTML writer, process both the HTML and the plain text nodes, as we're collecting
        // the plainTextContent and the htmlContent
        if (mTextPart) {
            mTextPart->copyContentFrom();
        }
        if (mHTMLPart) {
            mHTMLPart->copyContentFrom();
        }
        return;
    }

    if (viewHtml() && mHTMLPart) {
        mHTMLPart->copyContentFrom();
        mHTMLPart->html(decorate);
    } else if (mTextNode) {
        mTextPart->html(decorate);
    }
}

QString AlternativeMessagePart::text() const
{
    if (mTextPart) {
        return mTextPart->text();
    }
    return QString();
}

//-----CertMessageBlock----------------------

CertMessagePart::CertMessagePart(ObjectTreeParser* otp, KMime::Content* node, const Kleo::CryptoBackend::Protocol *cryptoProto, bool autoImport)
: MessagePart(otp, QString())
, mAutoImport(autoImport)
, mCryptoProto(cryptoProto)
{
    if (!mNode) {
        qCWarning(MESSAGEVIEWER_LOG) << "not a valid node";
        return;
    }

    if (!mAutoImport) {
        return;
    }

    const QByteArray certData = node->decodedContent();

    Kleo::ImportJob *import = mCryptoProto->importJob();
    KleoJobExecutor executor;
    mImportResult = executor.exec(import, certData);
}

CertMessagePart::~CertMessagePart()
{

}

void CertMessagePart::html(bool decorate)
{
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }
    mOtp->writeCertificateImportResult(mImportResult);
}

QString CertMessagePart::text() const
{
    return QString();
}

//-----CryptMessageBlock---------------------

CryptoMessagePart::CryptoMessagePart(ObjectTreeParser *otp,
                                     const QString &text,
                                     const Kleo::CryptoBackend::Protocol *cryptoProto,
                                     const QString &fromAddress,
                                     KMime::Content *node)
    : MessagePart(otp, text)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mNode(node)
    , mDecryptMessage(false)
{
    mMetaData.technicalProblem = (mCryptoProto == 0);
    mMetaData.isSigned = false;
    mMetaData.isGoodSignature = false;
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;
    mMetaData.keyTrust = GpgME::Signature::Unknown;
    mMetaData.status = i18n("Wrong Crypto Plug-In.");
    mMetaData.status_code = GPGME_SIG_STAT_NONE;
}

CryptoMessagePart::~CryptoMessagePart()
{

}

void CryptoMessagePart::startDecryption(const QByteArray &text, const QTextCodec *aCodec)
{
    mDecryptMessage = true;

    KMime::Content *content = new KMime::Content;
    content->setBody(text);
    content->parse();

    startDecryption(content);

    if (!mMetaData.inProgress && mMetaData.isDecryptable) {
        setText(aCodec->toUnicode(mDecryptedData));
    }
}

void CryptoMessagePart::startDecryption(KMime::Content *data)
{
    if (!mNode && !data) {
        return;
    }

    if (!data) {
        data = mNode;
    }

    mDecryptMessage = true;

    bool signatureFound;
    bool actuallyEncrypted = true;
    bool decryptionStarted;

    bool bOkDecrypt = mOtp->okDecryptMIME(*data,
                                          mDecryptedData,
                                          signatureFound,
                                          mSignatures,
                                          true,
                                          mPassphraseError,
                                          actuallyEncrypted,
                                          decryptionStarted,
                                          mMetaData);
    if (decryptionStarted) {
        mMetaData.inProgress = true;
        return;
    }
    mMetaData.isDecryptable = bOkDecrypt;
    mMetaData.isEncrypted = actuallyEncrypted;
    mMetaData.isSigned = signatureFound;

    if (!mMetaData.isDecryptable) {
        setText(QString::fromUtf8(mDecryptedData.constData()));
    }

    if (mMetaData.isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, mMetaData, GpgME::Key());
        mVerifiedText = mDecryptedData;
    }

    if (mMetaData.isEncrypted && !mDecryptMessage) {
        mMetaData.isDecryptable = true;
    }

    if (mNode) {
        mOtp->mNodeHelper->setPartMetaData(mNode, mMetaData);

        if (mDecryptMessage) {
            auto tempNode = new KMime::Content();
            tempNode->setContent(KMime::CRLFtoLF(mDecryptedData.constData()));
            tempNode->parse();

            if (!tempNode->head().isEmpty()) {
                tempNode->contentDescription()->from7BitString("encrypted data");
            }
            mOtp->mNodeHelper->attachExtraContent(mNode, tempNode);

            parseInternal(tempNode, false);
        }
    }
}

void CryptoMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    startVerificationDetached(text, 0, QByteArray());

    if (!mNode && mMetaData.isSigned) {
        setText(aCodec->toUnicode(mVerifiedText));
    }
}

void CryptoMessagePart::startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature)
{
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;

    mOtp->okVerify(text, mCryptoProto, mMetaData, mVerifiedText, mSignatures, signature, mNode);

    if (mMetaData.isSigned) {
        mOtp->sigStatusToMetaData(mSignatures, mCryptoProto, mMetaData, GpgME::Key());
    } else {
        mMetaData.creationTime = QDateTime();
    }

    if (mNode) {
        if (textNode && !signature.isEmpty()) {
            mVerifiedText = text;
        } else if (!mVerifiedText.isEmpty()) {
            textNode = new KMime::Content();
            textNode->setContent(KMime::CRLFtoLF(mVerifiedText.constData()));
            textNode->parse();

            if (!textNode->head().isEmpty()) {
                textNode->contentDescription()->from7BitString("opaque signed data");
            }
            mOtp->mNodeHelper->attachExtraContent(mNode, textNode);
        }

        if (!mVerifiedText.isEmpty() && textNode) {
            parseInternal(textNode, false);
        }
    }

}

void CryptoMessagePart::writeDeferredDecryptionBlock() const
{
    Q_ASSERT(!mMetaData.isEncrypted);
    Q_ASSERT(mDecryptMessage);

    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();
    if (!writer) {
        return;
    }

    const QString iconName = QLatin1String("file:///") + KIconLoader::global()->iconPath(QStringLiteral("document-decrypt"),
                             KIconLoader::Small);
    writer->queue(QLatin1String("<div style=\"font-size:large; text-align:center;"
                                "padding-top:20pt;\">")
                  + i18n("This message is encrypted.")
                  + QLatin1String("</div>"
                                  "<div style=\"text-align:center; padding-bottom:20pt;\">"
                                  "<a href=\"kmail:decryptMessage\">"
                                  "<img src=\"") + iconName + QLatin1String("\"/>")
                  + i18n("Decrypt Message")
                  + QLatin1String("</a></div>"));
}

void CryptoMessagePart::html(bool decorate)
{

    bool hideErrors = false;
    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    //TODO: still the following part should not be here
    copyContentFrom();

    if (!writer) {
        return;
    }

    if (mMetaData.isEncrypted && !mDecryptMessage) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        writeDeferredDecryptionBlock();
    } else if (mMetaData.inProgress) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        // In progress has no special body
    } else if (mMetaData.isEncrypted && !mMetaData.isDecryptable) {
        const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
        writer->queue(text());           //Do not quote ErrorText
    } else {
        if (mMetaData.isSigned && mVerifiedText.isEmpty() && !hideErrors) {
            const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
            writer->queue(QStringLiteral("<hr><b><h2>"));
            writer->queue(i18n("The crypto engine returned no cleartext data."));
            writer->queue(QStringLiteral("</h2></b>"));
            writer->queue(QStringLiteral("<br/>&nbsp;<br/>"));
            writer->queue(i18n("Status: "));
            if (!mMetaData.status.isEmpty()) {
                writer->queue(QStringLiteral("<i>"));
                writer->queue(mMetaData.status);
                writer->queue(QStringLiteral("</i>"));
            } else {
                writer->queue(i18nc("Status of message unknown.", "(unknown)"));
            }
        } else if (mNode) {
            const CryptoBlock block(mOtp, &mMetaData, mCryptoProto, mFromAddress, mNode);
            renderInternalHtml();
        } else {
            MessagePart::html(decorate);
        }
    }
}

EncapsulatedRfc822MessagePart::EncapsulatedRfc822MessagePart(ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message)
    : MessagePart(otp, QString())
    , mMessage(message)
    , mNode(node)
{
    mMetaData.isEncrypted = false;
    mMetaData.isSigned = false;
    mMetaData.isEncapsulatedRfc822Message = true;

    mOtp->nodeHelper()->setNodeDisplayedEmbedded(mNode, true);
    mOtp->nodeHelper()->setPartMetaData(mNode, mMetaData);

    if (!mMessage) {
        qCWarning(MESSAGEVIEWER_LOG) << "Node is of type message/rfc822 but doesn't have a message!";
        return;
    }

    // The link to "Encapsulated message" is clickable, therefore the temp file needs to exists,
    // since the user can click the link and expect to have normal attachment operations there.
    mOtp->nodeHelper()->writeNodeToTempFile(message.data());

    parseInternal(message.data(), false);
}

EncapsulatedRfc822MessagePart::~EncapsulatedRfc822MessagePart()
{

}

void EncapsulatedRfc822MessagePart::html(bool decorate)
{
    Q_UNUSED(decorate)
    if (!mSubOtp) {
        return;
    }

    MessageViewer::HtmlWriter *writer = mOtp->htmlWriter();

    if (!writer) {
        return;
    }

    const CryptoBlock block(mOtp, &mMetaData, Q_NULLPTR, mMessage->from()->asUnicodeString(), mMessage.data());
    writer->queue(mOtp->mSource->createMessageHeader(mMessage.data()));
    renderInternalHtml();

    mOtp->nodeHelper()->setPartMetaData(mNode, mMetaData);
}

QString EncapsulatedRfc822MessagePart::text() const
{
    return renderInternalText();
}

