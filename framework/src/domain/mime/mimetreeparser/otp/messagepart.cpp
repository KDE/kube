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
#include "mimetreeparser_debug.h"
#include "cryptohelper.h"
#include "objecttreeparser.h"
#include "qgpgmejobexecutor.h"

#include "cryptobodypartmemento.h"
#include "decryptverifybodypartmemento.h"
#include "verifydetachedbodypartmemento.h"
#include "verifyopaquebodypartmemento.h"

#include "utils.h"

#include <KMime/Content>

#include <QGpgME/DN>
#include <QGpgME/Protocol>
#include <QGpgME/ImportJob>
#include <QGpgME/KeyListJob>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/VerifyOpaqueJob>

#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>
#include <gpgme.h>

#include <KLocalizedString>

#include <QTextCodec>

using namespace MimeTreeParser;

//------MessagePart-----------------------
MessagePart::MessagePart(ObjectTreeParser *otp, const QString &text, KMime::Content *node)
    : mText(text)
    , mOtp(otp)
    , mNode(node) //only null for messagepartlist
    , mParentPart(nullptr)
    , mRoot(false)
{
}

MessagePart::~MessagePart()
{
}

/*
QByteArray MailMime::cid() const
{
    if (!d->mNode || !d->mNode->contentID()) {
        return QByteArray();
    }
    return d->mNode->contentID()->identifier();
}
*/

/*
bool MailMime::isFirstTextPart() const
{
    if (!d->mNode || !d->mNode->topLevel()) {
        return false;
    }
    return (d->mNode->topLevel()->textContent() == d->mNode);
}

bool MailMime::isFirstPart() const
{
   if (!d->mNode || !d->mNode->parent()) {
       return false;
   }
   return (d->mNode->parent()->contents().first() == d->mNode);
}

bool MailMime::isTopLevelPart() const
{
    if (!d->mNode) {
        return false;
    }
    return (d->mNode->topLevel() == d->mNode);
}
*/

MessagePart::Disposition MessagePart::disposition() const
{
    if (!mNode) {
        return Invalid;
    }
    const auto cd = mNode->contentDisposition(false);
    if (!cd) {
        return Invalid;
    }
    switch (cd->disposition()){
        case KMime::Headers::CDinline:
            return Inline;
        case KMime::Headers::CDattachment:
            return Attachment;
        default:
            return Invalid;
    }
}

QString MessagePart::filename() const
{
    if (!mNode) {
        return QString();
    }
    const auto cd = mNode->contentDisposition(false);
    if (!cd) {
        return QString();
    }
    return cd->filename();
}

static KMime::Headers::ContentType *contentType(KMime::Content *node)
{
    if (node) {
        return node->contentType(false);
    }
    return nullptr;
}

QByteArray MessagePart::charset() const
{
    if (auto ct = contentType(mNode)) {
        return ct->charset();
    }
    return mNode->defaultCharset();
}

QByteArray MessagePart::mimeType() const
{
    if (auto ct = contentType(mNode)) {
        return ct->mimeType();
    }
    return {};
}

bool MessagePart::isText() const
{
    if (auto ct = contentType(mNode)) {
        return ct->isText();
    }
    return false;
}

PartMetaData *MessagePart::partMetaData()
{
    return &mMetaData;
}

bool MessagePart::isAttachment() const
{
    return true;
}

KMime::Content *MessagePart::node() const
{
    return mNode;
}

void MessagePart::setIsRoot(bool root)
{
    mRoot = root;
}

bool MessagePart::isRoot() const
{
    return mRoot;
}

QString MessagePart::text() const
{
    return mText;
}

void MessagePart::setText(const QString &text)
{
    mText = text;
}

bool MessagePart::isHtml() const
{
    return false;
}

MessagePart *MessagePart::parentPart() const
{
    return mParentPart;
}

void MessagePart::setParentPart(MessagePart *parentPart)
{
    mParentPart = parentPart;
}

QString MessagePart::htmlContent() const
{
    return text();
}

QString MessagePart::plaintextContent() const
{
    return text();
}



void MessagePart::parseInternal(KMime::Content *node, bool onlyOneMimePart)
{
    auto subMessagePart = mOtp->parseObjectTreeInternal(node, onlyOneMimePart);
    mRoot = subMessagePart->isRoot();
    foreach (const auto &part, subMessagePart->subParts()) {
        appendSubPart(part);
    }
}

QString MessagePart::renderInternalText() const
{
    QString text;
    foreach (const auto &mp, subParts()) {
        text += mp->text();
    }
    return text;
}

void MessagePart::appendSubPart(const MessagePart::Ptr &messagePart)
{
    messagePart->setParentPart(this);
    mBlocks.append(messagePart);
}

const QVector<MessagePart::Ptr> &MessagePart::subParts() const
{
    return mBlocks;
}

bool MessagePart::hasSubParts() const
{
    return !mBlocks.isEmpty();
}

QVector<SignedMessagePart*> MessagePart::signatures() const
{
    QVector<SignedMessagePart*> list;
    auto parent = parentPart();
    while (parent) {
        if (auto sig = dynamic_cast<SignedMessagePart*>(parent)) {
            list << sig;
        }
        parent = parent->parentPart();
    }
    return list;
}

QVector<EncryptedMessagePart*> MessagePart::encryptions() const
{
    QVector<EncryptedMessagePart*> list;
    auto parent = parentPart();
    while (parent) {
        if (auto sig = dynamic_cast<EncryptedMessagePart*>(parent)) {
            list << sig;
        }
        parent = parent->parentPart();
    }
    return list;
}

//-----MessagePartList----------------------
MessagePartList::MessagePartList(ObjectTreeParser *otp, KMime::Content *node)
    : MessagePart(otp, QString(), node)
{
}

MessagePartList::~MessagePartList()
{

}

QString MessagePartList::text() const
{
    return renderInternalText();
}

QString MessagePartList::plaintextContent() const
{
    return QString();
}

QString MessagePartList::htmlContent() const
{
    return QString();
}

//-----TextMessageBlock----------------------

TextMessagePart::TextMessagePart(ObjectTreeParser *otp, KMime::Content *node)
    : MessagePartList(otp, node)
{
    if (!mNode) {
        qCWarning(MIMETREEPARSER_LOG) << "not a valid node";
        return;
    }


    parseContent();
}

TextMessagePart::~TextMessagePart()
{

}

void TextMessagePart::parseContent()
{
    const auto aCodec = mOtp->codecFor(mNode);
    const QString &fromAddress = mOtp->nodeHelper()->fromAsString(mNode);
    mSignatureState  = KMMsgNotSigned;
    mEncryptionState = KMMsgNotEncrypted;
    const auto blocks = prepareMessageForDecryption(mNode->decodedContent());

    const auto cryptProto = QGpgME::openpgp();

    if (!blocks.isEmpty()) {

        /* The (overall) signature/encrypted status is broken
         * if one unencrypted part is at the beginning or in the middle
         * because mailmain adds an unencrypted part at the end this should not break the overall status
         *
         * That's why we first set the tmp status and if one crypted/signed block comes afterwards, than
         * the status is set to unencryped
         */
        bool fullySignedOrEncrypted = true;
        bool fullySignedOrEncryptedTmp = true;

        for (const auto &block : blocks) {

            if (!fullySignedOrEncryptedTmp) {
                fullySignedOrEncrypted = false;
            }

            if (block.type() == NoPgpBlock && !block.text().trimmed().isEmpty()) {
                fullySignedOrEncryptedTmp = false;
                appendSubPart(MessagePart::Ptr(new MessagePart(mOtp, aCodec->toUnicode(block.text()))));
            } else if (block.type() == PgpMessageBlock) {
                KMime::Content *content = new KMime::Content;
                content->setBody(block.text());
                content->parse();
                EncryptedMessagePart::Ptr mp(new EncryptedMessagePart(mOtp, QString(), cryptProto, fromAddress, nullptr, content));
                mp->setIsEncrypted(true);
                appendSubPart(mp);
                continue;
            } else if (block.type() == ClearsignedBlock) {
                KMime::Content *content = new KMime::Content;
                content->setBody(block.text());
                content->parse();
                SignedMessagePart::Ptr mp(new SignedMessagePart(mOtp, QString(), cryptProto, fromAddress, nullptr, content));
                mp->setIsSigned(true);
                appendSubPart(mp);
                continue;
            } else {
                continue;
            }

            const auto mp = subParts().last().staticCast<MessagePart>();
            const PartMetaData *messagePart(mp->partMetaData());

            if (!messagePart->isEncrypted && !messagePart->isSigned && !block.text().trimmed().isEmpty()) {
                mp->setText(aCodec->toUnicode(block.text()));
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

KMMsgEncryptionState TextMessagePart::encryptionState() const
{
    return mEncryptionState;
}

KMMsgSignatureState TextMessagePart::signatureState() const
{
    return mSignatureState;
}

//-----AttachmentMessageBlock----------------------

AttachmentMessagePart::AttachmentMessagePart(ObjectTreeParser *otp, KMime::Content *node)
    : TextMessagePart(otp, node)
{

}

AttachmentMessagePart::~AttachmentMessagePart()
{

}


//-----HtmlMessageBlock----------------------

HtmlMessagePart::HtmlMessagePart(ObjectTreeParser *otp, KMime::Content *node)
    : MessagePart(otp, QString(), node)
{
    if (!mNode) {
        qCWarning(MIMETREEPARSER_LOG) << "not a valid node";
        return;
    }

    const QByteArray partBody(mNode->decodedContent());
    mBodyHTML = mOtp->codecFor(mNode)->toUnicode(partBody);
}

HtmlMessagePart::~HtmlMessagePart()
{
}

QString HtmlMessagePart::text() const
{
    return mBodyHTML;
}

bool HtmlMessagePart::isHtml() const
{
    return true;
}

//-----MimeMessageBlock----------------------

MimeMessagePart::MimeMessagePart(ObjectTreeParser *otp, KMime::Content *node, bool onlyOneMimePart)
    : MessagePart(otp, QString(), node)
{
    if (!mNode) {
        qCWarning(MIMETREEPARSER_LOG) << "not a valid node";
        return;
    }

    parseInternal(mNode, onlyOneMimePart);
}

MimeMessagePart::~MimeMessagePart()
{

}

QString MimeMessagePart::text() const
{
    return renderInternalText();
}

QString MimeMessagePart::plaintextContent() const
{
    return QString();
}

QString MimeMessagePart::htmlContent() const
{
    return QString();
}

//-----AlternativeMessagePart----------------------

AlternativeMessagePart::AlternativeMessagePart(ObjectTreeParser *otp, KMime::Content *node, Util::HtmlMode preferredMode)
    : MessagePart(otp, QString(), node)
    , mPreferredMode(preferredMode)
{
    KMime::Content *dataIcal = findTypeInDirectChilds(mNode, "text/calendar");
    KMime::Content *dataHtml = findTypeInDirectChilds(mNode, "text/html");
    KMime::Content *dataText = findTypeInDirectChilds(mNode, "text/plain");

    if (!dataHtml) {
        // If we didn't find the HTML part as the first child of the multipart/alternative, it might
        // be that this is a HTML message with images, and text/plain and multipart/related are the
        // immediate children of this multipart/alternative node.
        // In this case, the HTML node is a child of multipart/related.
        dataHtml = findTypeInDirectChilds(mNode, "multipart/related");

        // Still not found? Stupid apple mail actually puts the attachments inside of the
        // multipart/alternative, which is wrong. Therefore we also have to look for multipart/mixed
        // here.
        // Do this only when prefering HTML mail, though, since otherwise the attachments are hidden
        // when displaying plain text.
        if (!dataHtml) {
            dataHtml  = findTypeInDirectChilds(mNode, "multipart/mixed");
        }
    }

    if (dataIcal) {
        mChildNodes[Util::MultipartIcal] = dataIcal;
    }

    if (dataText) {
        mChildNodes[Util::MultipartPlain] = dataText;
    }

    if (dataHtml) {
        mChildNodes[Util::MultipartHtml] = dataHtml;
    }

    if (mChildNodes.isEmpty()) {
        qCWarning(MIMETREEPARSER_LOG) << "no valid nodes";
        return;
    }

    QMapIterator<Util::HtmlMode, KMime::Content *> i(mChildNodes);
    while (i.hasNext()) {
        i.next();
        mChildParts[i.key()] = MimeMessagePart::Ptr(new MimeMessagePart(mOtp, i.value(), true));
    }
}

AlternativeMessagePart::~AlternativeMessagePart()
{

}

Util::HtmlMode AlternativeMessagePart::preferredMode() const
{
    return mPreferredMode;
}

QList<Util::HtmlMode> AlternativeMessagePart::availableModes()
{
    return mChildParts.keys();
}

QString AlternativeMessagePart::text() const
{
    if (mChildParts.contains(Util::MultipartPlain)) {
        return mChildParts[Util::MultipartPlain]->text();
    }
    return QString();
}

bool AlternativeMessagePart::isHtml() const
{
    return mChildParts.contains(Util::MultipartHtml);
}

QString AlternativeMessagePart::plaintextContent() const
{
    return text();
}

QString AlternativeMessagePart::htmlContent() const
{
    if (mChildParts.contains(Util::MultipartHtml)) {
        return mChildParts[Util::MultipartHtml]->text();
    } else {
        return plaintextContent();
    }
}

//-----CertMessageBlock----------------------

CertMessagePart::CertMessagePart(ObjectTreeParser *otp, KMime::Content *node, const QGpgME::Protocol *cryptoProto)
    : MessagePart(otp, QString(), node)
    , mCryptoProto(cryptoProto)
{
    if (!mNode) {
        qCWarning(MIMETREEPARSER_LOG) << "not a valid node";
        return;
    }
}

CertMessagePart::~CertMessagePart()
{

}

void CertMessagePart::import()
{
    const QByteArray certData = mNode->decodedContent();
    QGpgME::ImportJob *import = mCryptoProto->importJob();
    QGpgMEJobExecutor executor;
    auto result = executor.exec(import, certData);
}

QString CertMessagePart::text() const
{
    return QString();
}

//-----SignedMessageBlock---------------------
SignedMessagePart::SignedMessagePart(ObjectTreeParser *otp,
                                     const QString &text,
                                     const QGpgME::Protocol *cryptoProto,
                                     const QString &fromAddress,
                                     KMime::Content *node, KMime::Content *signedData)
    : MessagePart(otp, text, node)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mSignedData(signedData)
{
    mMetaData.technicalProblem = (mCryptoProto == nullptr);
    mMetaData.isSigned = true;
    mMetaData.isGoodSignature = false;
    mMetaData.keyTrust = GpgME::Signature::Unknown;
    mMetaData.status = i18n("Wrong Crypto Plug-In.");
    mMetaData.status_code = GPGME_SIG_STAT_NONE;
}

SignedMessagePart::~SignedMessagePart()
{

}

void SignedMessagePart::setIsSigned(bool isSigned)
{
    mMetaData.isSigned = isSigned;
}

bool SignedMessagePart::isSigned() const
{
    return mMetaData.isSigned;
}

bool SignedMessagePart::okVerify(const QByteArray &data, const QByteArray &signature, KMime::Content *textNode)
{
    NodeHelper *nodeHelper = mOtp->nodeHelper();

    mMetaData.isSigned = false;
    mMetaData.technicalProblem = (mCryptoProto == nullptr);
    mMetaData.keyTrust = GpgME::Signature::Unknown;
    mMetaData.status = i18n("Wrong Crypto Plug-In.");
    mMetaData.status_code = GPGME_SIG_STAT_NONE;

    const QByteArray mementoName = "verification";

    //TODO for the async case remember the memento
    CryptoBodyPartMemento *m = nullptr;
    Q_ASSERT(!m || mCryptoProto); //No CryptoPlugin and having a bodyPartMemento -> there is something completely wrong

    if (!m && mCryptoProto) {
        if (!signature.isEmpty()) {
            QGpgME::VerifyDetachedJob *job = mCryptoProto->verifyDetachedJob();
            if (job) {
                m = new VerifyDetachedBodyPartMemento(job, mCryptoProto->keyListJob(), signature, data);
            }
        } else {
            QGpgME::VerifyOpaqueJob *job = mCryptoProto->verifyOpaqueJob();
            if (job) {
                m = new VerifyOpaqueBodyPartMemento(job, mCryptoProto->keyListJob(), data);
            }
        }
        if (m) {
            if (mOtp->allowAsync()) {
                QObject::connect(m, &CryptoBodyPartMemento::update,
                                 nodeHelper, &NodeHelper::update);
                // QObject::connect(m, SIGNAL(update(MimeTreeParser::UpdateMode)),
                //                  _source->sourceObject(), SLOT(update(MimeTreeParser::UpdateMode)));

                if (m->start()) {
                    mMetaData.inProgress = true;
                    mOtp->mHasPendingAsyncJobs = true;
                }
                //FIXME delete memento once done
            } else {
                m->exec();
            }
        }
    //only relevant in async case
    // } else if (m->isRunning()) {
    //     mMetaData.inProgress = true;
    //     mOtp->mHasPendingAsyncJobs = true;
    // } else {
    //     mMetaData.inProgress = false;
    //     mOtp->mHasPendingAsyncJobs = false;
    }

    if (m && !mMetaData.inProgress) {
        if (!signature.isEmpty()) {
            mVerifiedText = data;
        }
        setVerificationResult(m, textNode);
    }

    if (!m && !mMetaData.inProgress) {
        QString errorMsg;
        QString cryptPlugLibName;
        QString cryptPlugDisplayName;
        if (mCryptoProto) {
            cryptPlugLibName = mCryptoProto->name();
            cryptPlugDisplayName = mCryptoProto->displayName();
        }

        if (!mCryptoProto) {
            if (cryptPlugDisplayName.isEmpty()) {
                errorMsg = i18n("No appropriate crypto plug-in was found.");
            } else {
                errorMsg = i18nc("%1 is either 'OpenPGP' or 'S/MIME'",
                                 "No %1 plug-in was found.",
                                 cryptPlugDisplayName);
            }
        } else {
            errorMsg = i18n("Crypto plug-in \"%1\" cannot verify signatures.",
                            cryptPlugLibName);
        }
        mMetaData.errorText = i18n("The message is signed, but the "
                                   "validity of the signature cannot be "
                                   "verified.<br />"
                                   "Reason: %1",
                                   errorMsg);
    }
    //TODO don't delete in async case
    if (m) {
        delete m;
    }

    return mMetaData.isSigned;
}

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

QString prettifyDN(const char *uid)
{
    return QGpgME::DN(uid).prettyDN();
}

void SignedMessagePart::sigStatusToMetaData()
{
    GpgME::Key key;
    if (mMetaData.isSigned) {
        GpgME::Signature signature = mSignatures.front();
        mMetaData.status_code = signatureToStatus(signature);
        mMetaData.isGoodSignature = mMetaData.status_code & GPGME_SIG_STAT_GOOD;
        // save extended signature status flags
        mMetaData.sigSummary = signature.summary();

        if (mMetaData.isGoodSignature && !key.keyID()) {
            // Search for the key by its fingerprint so that we can check for
            // trust etc.
            QGpgME::KeyListJob *job = mCryptoProto->keyListJob(false);    // local, no sigs
            if (!job) {
                qCDebug(MIMETREEPARSER_LOG) << "The Crypto backend does not support listing keys. ";
            } else {
                std::vector<GpgME::Key> found_keys;
                // As we are local it is ok to make this synchronous
                GpgME::KeyListResult res = job->exec(QStringList(QLatin1String(signature.fingerprint())), false, found_keys);
                if (res.error()) {
                    qCDebug(MIMETREEPARSER_LOG) << "Error while searching key for Fingerprint: " << signature.fingerprint();
                }
                if (found_keys.size() > 1) {
                    // Should not Happen
                    qCDebug(MIMETREEPARSER_LOG) << "Oops: Found more then one Key for Fingerprint: " << signature.fingerprint();
                }
                if (found_keys.size() != 1) {
                    // Should not Happen at this point
                    qCDebug(MIMETREEPARSER_LOG) << "Oops: Found no Key for Fingerprint: " << signature.fingerprint();
                } else {
                    key = found_keys[0];
                }
                delete job;
            }
        }

        if (key.keyID()) {
            mMetaData.keyId = key.keyID();
        }
        if (mMetaData.keyId.isEmpty()) {
            mMetaData.keyId = signature.fingerprint();
        }
        mMetaData.keyTrust = signature.validity();
        if (key.numUserIDs() > 0 && key.userID(0).id()) {
            mMetaData.signer = prettifyDN(key.userID(0).id());
        }
        for (uint iMail = 0; iMail < key.numUserIDs(); ++iMail) {
            // The following if /should/ always result in TRUE but we
            // won't trust implicitely the plugin that gave us these data.
            if (key.userID(iMail).email()) {
                QString email = QString::fromUtf8(key.userID(iMail).email());
                // ### work around gpgme 0.3.QString text() const Q_DECL_OVERRIDE;x / cryptplug bug where the
                // ### email addresses are specified as angle-addr, not addr-spec:
                if (email.startsWith(QLatin1Char('<')) && email.endsWith(QLatin1Char('>'))) {
                    email = email.mid(1, email.length() - 2);
                }
                if (!email.isEmpty()) {
                    mMetaData.signerMailAddresses.append(email);
                }
            }
        }

        if (signature.creationTime()) {
            mMetaData.creationTime.setTime_t(signature.creationTime());
        } else {
            mMetaData.creationTime = QDateTime();
        }
        if (mMetaData.signer.isEmpty()) {
            if (key.numUserIDs() > 0 && key.userID(0).name()) {
                mMetaData.signer = prettifyDN(key.userID(0).name());
            }
            if (!mMetaData.signerMailAddresses.empty()) {
                if (mMetaData.signer.isEmpty()) {
                    mMetaData.signer = mMetaData.signerMailAddresses.front();
                } else {
                    mMetaData.signer += QLatin1String(" <") + mMetaData.signerMailAddresses.front() + QLatin1Char('>');
                }
            }
        }
    }
}

void SignedMessagePart::startVerification()
{
    if (mSignedData) {
        const QByteArray cleartext = KMime::LFtoCRLF(mSignedData->encodedContent());
        const QTextCodec *aCodec(mOtp->codecFor(mSignedData));

        //The case for pkcs7
        if (mNode == mSignedData) {
            startVerificationDetached(cleartext, nullptr, {});
        } else {
            startVerificationDetached(cleartext, mSignedData, mNode->decodedContent());
        }
    }
}

void SignedMessagePart::startVerification(const QByteArray &text, const QTextCodec *aCodec)
{
    startVerificationDetached(text, nullptr, QByteArray());

    if (!mNode && mMetaData.isSigned) {
        setText(aCodec->toUnicode(mVerifiedText));
    }
}

void SignedMessagePart::startVerificationDetached(const QByteArray &text, KMime::Content *textNode, const QByteArray &signature)
{
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;

    if (textNode) {
        parseInternal(textNode, false);
    }

    okVerify(text, signature, textNode);

    if (!mMetaData.isSigned) {
        mMetaData.creationTime = QDateTime();
    }
}

void SignedMessagePart::setVerificationResult(const CryptoBodyPartMemento *m, KMime::Content *textNode)
{
    if (const auto vm = dynamic_cast<const VerifyDetachedBodyPartMemento *>(m)) {
        mSignatures = vm->verifyResult().signatures();
    }
    if (const auto vm = dynamic_cast<const VerifyOpaqueBodyPartMemento *>(m)) {
        mVerifiedText = vm->plainText();
        mSignatures = vm->verifyResult().signatures();
    }
    if (const auto vm = dynamic_cast<const DecryptVerifyBodyPartMemento *>(m)) {
        mVerifiedText = vm->plainText();
        mSignatures = vm->verifyResult().signatures();
    }
    mMetaData.auditLogError = m->auditLogError();
    mMetaData.auditLog = m->auditLogAsHtml();
    mMetaData.isSigned = !mSignatures.empty();

    if (mMetaData.isSigned) {
        sigStatusToMetaData();
        if (mNode) {
            mOtp->nodeHelper()->setSignatureState(mNode, KMMsgFullySigned);
            if (!textNode) {
                mOtp->mNodeHelper->setPartMetaData(mNode, mMetaData);

                if (!mVerifiedText.isEmpty()) {
                    auto tempNode = new KMime::Content();
                    tempNode->setContent(KMime::CRLFtoLF(mVerifiedText.constData()));
                    tempNode->parse();

                    if (!tempNode->head().isEmpty()) {
                        tempNode->contentDescription()->from7BitString("signed data");
                    }
                    mOtp->mNodeHelper->attachExtraContent(mNode, tempNode);

                    parseInternal(tempNode, false);
                }
            }
        }
    }
}

QString SignedMessagePart::plaintextContent() const
{
    if (!mNode) {
        return MessagePart::text();
    } else {
        return QString();
    }
}

QString SignedMessagePart::htmlContent() const
{
    if (!mNode) {
        return MessagePart::text();
    } else {
        return QString();
    }
}

//-----CryptMessageBlock---------------------
EncryptedMessagePart::EncryptedMessagePart(ObjectTreeParser *otp,
        const QString &text,
        const QGpgME::Protocol *cryptoProto,
        const QString &fromAddress,
        KMime::Content *node, KMime::Content *encryptedNode)
    : MessagePart(otp, text, node)
    , mPassphraseError(false)
    , mNoSecKey(false)
    , mCryptoProto(cryptoProto)
    , mFromAddress(fromAddress)
    , mEncryptedNode(encryptedNode)
{
    mMetaData.technicalProblem = (mCryptoProto == nullptr);
    mMetaData.isSigned = false;
    mMetaData.isGoodSignature = false;
    mMetaData.isEncrypted = false;
    mMetaData.isDecryptable = false;
    mMetaData.keyTrust = GpgME::Signature::Unknown;
    mMetaData.status = i18n("Wrong Crypto Plug-In.");
    mMetaData.status_code = GPGME_SIG_STAT_NONE;
}

EncryptedMessagePart::~EncryptedMessagePart()
{

}

void EncryptedMessagePart::setIsEncrypted(bool encrypted)
{
    mMetaData.isEncrypted = encrypted;
}

bool EncryptedMessagePart::isEncrypted() const
{
    return mMetaData.isEncrypted;
}

bool EncryptedMessagePart::isDecryptable() const
{
    return mMetaData.isDecryptable;
}

bool EncryptedMessagePart::passphraseError() const
{
    return mPassphraseError;
}

void EncryptedMessagePart::startDecryption(const QByteArray &text, const QTextCodec *aCodec)
{
    KMime::Content *content = new KMime::Content;
    content->setBody(text);
    content->parse();

    startDecryption(content);

    auto code = aCodec ? aCodec : mOtp->codecFor(mNode);
    if (!mMetaData.inProgress && mMetaData.isDecryptable) {
        if (hasSubParts()) {
            auto _mp = (subParts()[0]).dynamicCast<SignedMessagePart>();
            if (_mp) {
                _mp->setText(aCodec->toUnicode(mDecryptedData));
            } else {
                setText(aCodec->toUnicode(mDecryptedData));
            }
        } else {
            setText(aCodec->toUnicode(mDecryptedData));
        }
    }
}

bool EncryptedMessagePart::okDecryptMIME(KMime::Content &data)
{
    mPassphraseError = false;
    mMetaData.inProgress = false;
    mMetaData.errorText.clear();
    mMetaData.auditLogError = GpgME::Error();
    mMetaData.auditLog.clear();
    bool bDecryptionOk = false;
    bool cannotDecrypt = false;
    NodeHelper *nodeHelper = mOtp->nodeHelper();

    // TODO in the async case remember the memento:
    const DecryptVerifyBodyPartMemento *m = nullptr;

    Q_ASSERT(!m || mCryptoProto); //No CryptoPlugin and having a bodyPartMemento -> there is something completely wrong

    if (!m && mCryptoProto) {
        QGpgME::DecryptVerifyJob *job = mCryptoProto->decryptVerifyJob();
        if (!job) {
            cannotDecrypt = true;
        } else {
            const QByteArray ciphertext = data.decodedContent();
            DecryptVerifyBodyPartMemento *newM = new DecryptVerifyBodyPartMemento(job, ciphertext);
            if (mOtp->allowAsync()) {
                QObject::connect(newM, &CryptoBodyPartMemento::update,
                                 nodeHelper, &NodeHelper::update);
                // QObject::connect(newM, SIGNAL(update(MimeTreeParser::UpdateMode)), _source->sourceObject(),
                //                  SLOT(update(MimeTreeParser::UpdateMode)));
                if (newM->start()) {
                    mMetaData.inProgress = true;
                    mOtp->mHasPendingAsyncJobs = true;
                } else {
                    m = newM;
                }
            } else {
                newM->exec();
                m = newM;
            }
        }
        //Only relevant in the async case
    // } else if (m->isRunning()) {
    //     mMetaData.inProgress = true;
    //     mOtp->mHasPendingAsyncJobs = true;
    //     m = nullptr;
    }

    if (m) {
        const QByteArray &plainText = m->plainText();
        const GpgME::DecryptionResult &decryptResult = m->decryptResult();
        const GpgME::VerificationResult &verifyResult = m->verifyResult();
        mMetaData.isSigned = verifyResult.signatures().size() > 0;

        if (verifyResult.signatures().size() > 0) {
            //We simply attach a signed message part to indicate that this content is also signed
            auto subPart = SignedMessagePart::Ptr(new SignedMessagePart(mOtp, QString::fromUtf8(plainText), mCryptoProto, mFromAddress, nullptr, nullptr));
            subPart->setVerificationResult(m, nullptr);
            appendSubPart(subPart);
        }

        mDecryptRecipients = decryptResult.recipients();
        bDecryptionOk = !decryptResult.error();
//        std::stringstream ss;
//        ss << decryptResult << '\n' << verifyResult;
//        qCDebug(MIMETREEPARSER_LOG) << ss.str().c_str();

        if (!bDecryptionOk && mMetaData.isSigned) {
            //Only a signed part
            mMetaData.isEncrypted = false;
            bDecryptionOk = true;
            mDecryptedData = plainText;
        } else {
            mPassphraseError =  decryptResult.error().isCanceled() || decryptResult.error().code() == GPG_ERR_NO_SECKEY;
            mMetaData.isEncrypted = decryptResult.error().code() != GPG_ERR_NO_DATA;
            mMetaData.errorText = QString::fromLocal8Bit(decryptResult.error().asString());
            if (mMetaData.isEncrypted && decryptResult.numRecipients() > 0) {
                mMetaData.keyId = decryptResult.recipient(0).keyID();
            }

            if (bDecryptionOk) {
                mDecryptedData = plainText;
                setText(QString::fromUtf8(mDecryptedData.constData()));
            } else {
                mNoSecKey = true;
                foreach (const GpgME::DecryptionResult::Recipient &recipient, decryptResult.recipients()) {
                    mNoSecKey &= (recipient.status().code() == GPG_ERR_NO_SECKEY);
                }
                if (!mPassphraseError && !mNoSecKey) {          // GpgME do not detect passphrase error correctly
                    mPassphraseError = true;
                }
            }
        }
    }

    if (!bDecryptionOk) {
        if (!mCryptoProto) {
            mMetaData.errorText = i18n("No appropriate crypto plug-in was found.");
        } else if (cannotDecrypt) {
            mMetaData.errorText = i18n("Crypto plug-in \"%1\" cannot decrypt messages.", mCryptoProto->name());
        } else if (!passphraseError()) {
            mMetaData.errorText = i18n("Crypto plug-in \"%1\" could not decrypt the data.", mCryptoProto->name())
                                  + i18n("Error: %1", mMetaData.errorText);
        }
    }
    //TODO don't delete in async case
    if (m) {
        delete m;
    }
    return bDecryptionOk;
}

void EncryptedMessagePart::startDecryption(KMime::Content *data)
{

    if (!data) {
        data = mEncryptedNode;
        if (!data) {
            data = mNode;
        }
    }

    mMetaData.isEncrypted = true;

    bool bOkDecrypt = okDecryptMIME(*data);

    if (mMetaData.inProgress) {
        return;
    }
    mMetaData.isDecryptable = bOkDecrypt;

    if (!mMetaData.isDecryptable) {
        setText(QString::fromUtf8(mDecryptedData.constData()));
    }

    // if (mMetaData.isEncrypted && !decryptMessage()) {
    //     mMetaData.isDecryptable = true;
    // }

    if (mNode && !mMetaData.isSigned) {
        mOtp->mNodeHelper->setPartMetaData(mNode, mMetaData);
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

QString EncryptedMessagePart::plaintextContent() const
{
    if (!mNode) {
        return MessagePart::text();
    } else {
        return QString();
    }
}

QString EncryptedMessagePart::htmlContent() const
{
    if (!mNode) {
        return MessagePart::text();
    } else {
        return QString();
    }
}

QString EncryptedMessagePart::text() const
{
    if (hasSubParts()) {
        auto _mp = (subParts()[0]).dynamicCast<SignedMessagePart>();
        if (_mp) {
            return _mp->text();
        } else {
            return MessagePart::text();
        }
    } else {
        return MessagePart::text();
    }
}

EncapsulatedRfc822MessagePart::EncapsulatedRfc822MessagePart(ObjectTreeParser *otp, KMime::Content *node, const KMime::Message::Ptr &message)
    : MessagePart(otp, QString(), node)
    , mMessage(message)
{
    mMetaData.isEncrypted = false;
    mMetaData.isSigned = false;
    mMetaData.isEncapsulatedRfc822Message = true;

    mOtp->nodeHelper()->setPartMetaData(mNode, mMetaData);

    if (!mMessage) {
        qCWarning(MIMETREEPARSER_LOG) << "Node is of type message/rfc822 but doesn't have a message!";
        return;
    }

    parseInternal(message.data(), false);
}

EncapsulatedRfc822MessagePart::~EncapsulatedRfc822MessagePart()
{

}

QString EncapsulatedRfc822MessagePart::text() const
{
    return renderInternalText();
}

