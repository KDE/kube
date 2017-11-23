/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "mailcrypto.h"
#include <QGpgME/Protocol>
#include <QGpgME/SignJob>
#include <QGpgME/EncryptJob>
#include <QGpgME/SignEncryptJob>
#include <QGpgME/KeyListJob>
#include <gpgme++/global.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/keylistresult.h>
#include <QDebug>

/*
 * FIXME:
 *
 * This code is WIP.
 * It currently only implements OpenPGPMIMEFormat for signing.
 * All the commented code are intentional leftovers that we can clean-up
 * once all necessary signing mechanisms have been implemented.
 *
 * Creating an ecrypted mail:
 * * get keys (email -> fingreprint -> key)
 * * Use Kleo::OpenPGPMIMEFormat,
 *
 */

// bool chooseCTE()
// {
//     Q_Q(SinglepartJob);

//     auto allowed = KMime::encodingsForData(data);

//     if (!q->globalPart()->is8BitAllowed()) {
//         allowed.removeAll(KMime::Headers::CE8Bit);
//     }

// #if 0 //TODO signing
//     // In the following cases only QP and Base64 are allowed:
//     // - the buffer will be OpenPGP/MIME signed and it contains trailing
//     //   whitespace (cf. RFC 3156)
//     // - a line starts with "From "
//     if ((willBeSigned && cf.hasTrailingWhitespace()) ||
//             cf.hasLeadingFrom()) {
//         ret.removeAll(DwMime::kCte8bit);
//         ret.removeAll(DwMime::kCte7bit);
//     }
// #endif

//     if (contentTransferEncoding) {
//         // Specific CTE set.  Check that our data fits in it.
//         if (!allowed.contains(contentTransferEncoding->encoding())) {
//             q->setError(JobBase::BugError);
//             q->setErrorText(i18n("%1 Content-Transfer-Encoding cannot correctly encode this message.",
//                                  KMime::nameForEncoding(contentTransferEncoding->encoding())));
//             return false;
//             // TODO improve error message in case 8bit is requested but not allowed.
//         }
//     } else {
//         // No specific CTE set.  Choose the best one.
//         Q_ASSERT(!allowed.isEmpty());
//         contentTransferEncoding = new KMime::Headers::ContentTransferEncoding;
//         contentTransferEncoding->setEncoding(allowed.first());
//     }
//     qCDebug(MESSAGECOMPOSER_LOG) << "Settled on encoding" << KMime::nameForEncoding(contentTransferEncoding->encoding());
//     return true;
// }

KMime::Content *createPart(const QByteArray &encodedBody, const QByteArray &mimeType, const QByteArray &charset)
{
    auto resultContent = new KMime::Content;

    auto contentType = new KMime::Headers::ContentType;
    contentType->setMimeType(mimeType);
    contentType->setMimeType(charset);
    // if (!chooseCTE()) {
    //     Q_ASSERT(error());
    //     emitResult();
    //     return;
    // }

    // Set headers.
    // if (contentDescription) {
    //     resultContent->setHeader(contentDescription);
    // }
    // if (contentDisposition) {
    //     resultContent->setHeader(contentDisposition);
    // }
    // if (contentID) {
    //     resultContent->setHeader(contentID);
    // }
    // Q_ASSERT(contentTransferEncoding);   // chooseCTE() created it if it didn't exist.
    auto contentTransferEncoding = new KMime::Headers::ContentTransferEncoding;
    auto allowed = KMime::encodingsForData(encodedBody);
    Q_ASSERT(!allowed.isEmpty());
    contentTransferEncoding->setEncoding(allowed.first());
    resultContent->setHeader(contentTransferEncoding);

    if (contentType) {
        resultContent->setHeader(contentType);
    }

    // Set data.
    resultContent->setBody(encodedBody);
    return resultContent;
}

KMime::Content *setBodyAndCTE(QByteArray &encodedBody, KMime::Headers::ContentType *contentType, KMime::Content *ret)
{
    // MessageComposer::Composer composer;
    // MessageComposer::SinglepartJob cteJob(&composer);
    auto part = createPart(encodedBody, contentType->mimeType(), contentType->charset());
    part->assemble();

    // cteJob.contentType()->setMimeType(contentType->mimeType());
    // cteJob.contentType()->setCharset(contentType->charset());
    // cteJob.setData(encodedBody);
    // cteJob.exec();
    // cteJob.content()->assemble();

    ret->contentTransferEncoding()->setEncoding(part->contentTransferEncoding()->encoding());
    ret->setBody(part->encodedBody());

    return ret;
}

void makeToplevelContentType(KMime::Content *content, bool sign, const QByteArray &hashAlgo)
{
    //Kleo::CryptoMessageFormat format, 
    // switch (format) {
    // default:
    // case Kleo::InlineOpenPGPFormat:
    // case Kleo::OpenPGPMIMEFormat:
        if (sign) {
            content->contentType()->setMimeType(QByteArrayLiteral("multipart/signed"));
            content->contentType()->setParameter(QStringLiteral("protocol"), QStringLiteral("application/pgp-signature"));
            content->contentType()->setParameter(QStringLiteral("micalg"), QString::fromLatin1(QByteArray(QByteArrayLiteral("pgp-") + hashAlgo)).toLower());

        } else {
            content->contentType()->setMimeType(QByteArrayLiteral("multipart/encrypted"));
            content->contentType()->setParameter(QStringLiteral("protocol"), QStringLiteral("application/pgp-encrypted"));
        }
        return;
    // case Kleo::SMIMEFormat:
    //     if (sign) {
    //         qCDebug(MESSAGECOMPOSER_LOG) << "setting headers for SMIME";
    //         content->contentType()->setMimeType(QByteArrayLiteral("multipart/signed"));
    //         content->contentType()->setParameter(QStringLiteral("protocol"), QString::fromAscii("application/pkcs7-signature"));
    //         content->contentType()->setParameter(QStringLiteral("micalg"), QString::fromAscii(hashAlgo).toLower());
    //         return;
    //     }
    // // fall through (for encryption, there's no difference between
    // // SMIME and SMIMEOpaque, since there is no mp/encrypted for
    // // S/MIME)
    // case Kleo::SMIMEOpaqueFormat:

    //     qCDebug(MESSAGECOMPOSER_LOG) << "setting headers for SMIME/opaque";
    //     content->contentType()->setMimeType(QByteArrayLiteral("application/pkcs7-mime"));

    //     if (sign) {
    //         content->contentType()->setParameter(QStringLiteral("smime-type"), QString::fromAscii("signed-data"));
    //     } else {
    //         content->contentType()->setParameter(QStringLiteral("smime-type"), QString::fromAscii("enveloped-data"));
    //     }
    //     content->contentType()->setParameter(QStringLiteral("name"), QString::fromAscii("smime.p7m"));
    // }
}

void setNestedContentType(KMime::Content *content, bool sign)
{
// , Kleo::CryptoMessageFormat format
    // switch (format) {
    // case Kleo::OpenPGPMIMEFormat:
        if (sign) {
            content->contentType()->setMimeType(QByteArrayLiteral("application/pgp-signature"));
            content->contentType()->setParameter(QStringLiteral("name"), QString::fromLatin1("signature.asc"));
            content->contentDescription()->from7BitString("This is a digitally signed message part.");
        } else {
            content->contentType()->setMimeType(QByteArrayLiteral("application/octet-stream"));
        }
        return;
    // case Kleo::SMIMEFormat:
    //     if (sign) {
    //         content->contentType()->setMimeType(QByteArrayLiteral("application/pkcs7-signature"));
    //         content->contentType()->setParameter(QStringLiteral("name"), QString::fromAscii("smime.p7s"));
    //         return;
    //     }
    // // fall through:
    // default:
    // case Kleo::InlineOpenPGPFormat:
    // case Kleo::SMIMEOpaqueFormat:
    //     ;
    // }
}

void setNestedContentDisposition(KMime::Content *content, bool sign)
{
// Kleo::CryptoMessageFormat format, 
    // if (!sign && format & Kleo::OpenPGPMIMEFormat) {
    if (!sign) {
        content->contentDisposition()->setDisposition(KMime::Headers::CDinline);
        content->contentDisposition()->setFilename(QStringLiteral("msg.asc"));
    // } else if (sign && format & Kleo::SMIMEFormat) {
    //     content->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    //     content->contentDisposition()->setFilename(QStringLiteral("smime.p7s"));
    }
}

// bool MessageComposer::Util::makeMultiMime(Kleo::CryptoMessageFormat format, bool sign)
// {
//     switch (format) {
//     default:
//     case Kleo::InlineOpenPGPFormat:
//     case Kleo::SMIMEOpaqueFormat:   return false;
//     case Kleo::OpenPGPMIMEFormat:   return true;
//     case Kleo::SMIMEFormat:         return sign; // only on sign - there's no mp/encrypted for S/MIME
//     }
// }

KMime::Content *composeHeadersAndBody(KMime::Content *orig, QByteArray encodedBody, bool sign, const QByteArray &hashAlgo)
{
    // Kleo::CryptoMessageFormat format,
    KMime::Content *result = new KMime::Content;

    // called should have tested that the signing/encryption failed
    Q_ASSERT(!encodedBody.isEmpty());

    // if (!(format & Kleo::InlineOpenPGPFormat)) {    // make a MIME message
        // qDebug() << "making MIME message, format:" << format;
        makeToplevelContentType(result, sign, hashAlgo);

        // if (makeMultiMime(sign)) {      // sign/enc PGPMime, sign SMIME
        if (true) {      // sign/enc PGPMime, sign SMIME

            const QByteArray boundary = KMime::multiPartBoundary();
            result->contentType()->setBoundary(boundary);

            result->assemble();
            //qCDebug(MESSAGECOMPOSER_LOG) << "processed header:" << result->head();

            // Build the encapsulated MIME parts.
            // Build a MIME part holding the code information
            // taking the body contents returned in ciphertext.
            KMime::Content *code = new KMime::Content;
            setNestedContentType(code, sign);
            setNestedContentDisposition(code, sign);

            if (sign) {                           // sign PGPMime, sign SMIME
                // if (format & Kleo::AnySMIME) {      // sign SMIME
                //     code->contentTransferEncoding()->setEncoding(KMime::Headers::CEbase64);
                //     code->contentTransferEncoding()->needToEncode();
                //     code->setBody(encodedBody);
                // } else {                            // sign PGPMmime
                    setBodyAndCTE(encodedBody, orig->contentType(), code);
                // }
                result->addContent(orig);
                result->addContent(code);
            } else {                              // enc PGPMime
                setBodyAndCTE(encodedBody, orig->contentType(), code);

                // Build a MIME part holding the version information
                // taking the body contents returned in
                // structuring.data.bodyTextVersion.
                KMime::Content *vers = new KMime::Content;
                vers->contentType()->setMimeType("application/pgp-encrypted");
                vers->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
                vers->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
                vers->setBody("Version: 1");

                result->addContent(vers);
                result->addContent(code);
            }
        } else {                                //enc SMIME, sign/enc SMIMEOpaque
            result->contentTransferEncoding()->setEncoding(KMime::Headers::CEbase64);
            result->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
            result->contentDisposition()->setFilename(QStringLiteral("smime.p7m"));

            result->assemble();
            //qCDebug(MESSAGECOMPOSER_LOG) << "processed header:" << result->head();

            result->setBody(encodedBody);
        }
    // } else {                                  // sign/enc PGPInline
    //     result->setHead(orig->head());
    //     result->parse();

    //     // fixing ContentTransferEncoding
    //     setBodyAndCTE(encodedBody, orig->contentType(), result);
    // }
    result->assemble();
    return result;
}

// bool binaryHint(Kleo::CryptoMessageFormat f)
// {
//     switch (f) {
//     case Kleo::SMIMEFormat:
//     case Kleo::SMIMEOpaqueFormat:
//         return true;
//     default:
//     case Kleo::OpenPGPMIMEFormat:
//     case Kleo::InlineOpenPGPFormat:
//         return false;
//     }
// }
//
    // GpgME::SignatureMode signingMode(Kleo::CryptoMessageFormat f)
    // {
    //     switch (f) {
    //     case Kleo::SMIMEOpaqueFormat:
    //         return GpgME::NormalSignatureMode;
    //     case Kleo::InlineOpenPGPFormat:
    //         return GpgME::Clearsigned;
    //     default:
    //     case Kleo::SMIMEFormat:
    //     case Kleo::OpenPGPMIMEFormat:
    //         return GpgME::Detached;
    //     }
    // }

// replace simple LFs by CRLFs for all MIME supporting CryptPlugs
// according to RfC 2633, 3.1.1 Canonicalization
static QByteArray canonicalizeContent(KMime::Content *content)
{
    // if (d->format & Kleo::InlineOpenPGPFormat) {
    //     return d->content->body();
    // } else if (!(d->format & Kleo::SMIMEOpaqueFormat)) {

        // replace "From " and "--" at the beginning of lines
        // with encoded versions according to RfC 3156, 3
        // Note: If any line begins with the string "From ", it is strongly
        //   suggested that either the Quoted-Printable or Base64 MIME encoding
        //   be applied.
        const auto encoding = content->contentTransferEncoding()->encoding();
        if ((encoding == KMime::Headers::CEquPr || encoding == KMime::Headers::CE7Bit)
                && !content->contentType(false)) {
            QByteArray body = content->encodedBody();
            bool changed = false;
            QList<QByteArray> search;
            QList<QByteArray> replacements;

            search       << "From "
                         << "from "
                         << "-";
            replacements << "From=20"
                         << "from=20"
                         << "=2D";

            if (content->contentTransferEncoding()->encoding() == KMime::Headers::CE7Bit) {
                for (int i = 0; i < search.size(); ++i) {
                    const auto pos = body.indexOf(search[i]);
                    if (pos == 0 || (pos > 0 && body.at(pos - 1) == '\n')) {
                        changed = true;
                        break;
                    }
                }
                if (changed) {
                    content->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
                    content->assemble();
                    body = content->encodedBody();
                }
            }

            for (int i = 0; i < search.size(); ++i) {
                const auto pos = body.indexOf(search[i]);
                if (pos == 0 || (pos > 0 && body.at(pos - 1) == '\n')) {
                    changed = true;
                    body.replace(pos, search[i].size(), replacements[i]);
                }
            }

            if (changed) {
                qDebug() << "Content changed";
                content->setBody(body);
                content->contentTransferEncoding()->setDecoded(false);
            }
        }

        return KMime::LFtoCRLF(content->encodedContent());
    // } else {                    // SMimeOpaque doesn't need LFtoCRLF, else it gets munged
    //     return content->encodedContent();
    // }

}

KMime::Content *MailCrypto::processCrypto(KMime::Content *content, const std::vector<GpgME::Key> &signingKeys, const std::vector<GpgME::Key> &encryptionKeys, MailCrypto::Protocol protocol)
{
    const QGpgME::Protocol *const proto = protocol == MailCrypto::SMIME ? QGpgME::smime() : QGpgME::openpgp();
    Q_ASSERT(proto);

    auto signingMode = GpgME::Detached;
    bool armor = true;
    bool textMode = false;
    const bool sign = !signingKeys.empty();
    const bool encrypt = !encryptionKeys.empty();

    QByteArray resultContent;
    QByteArray hashAlgo;
    //Trust provided keys and don't check them for validity
    bool alwaysTrust = true;
    if (sign && encrypt) {
        std::unique_ptr<QGpgME::SignEncryptJob> job(proto->signEncryptJob(armor, textMode));
        const auto res = job->exec(signingKeys, encryptionKeys, canonicalizeContent(content), alwaysTrust, resultContent);
        if (res.first.error().code()) {
            qWarning() << "Signing failed:" << res.first.error().asString();
            return nullptr;
        } else {
            hashAlgo = res.first.createdSignature(0).hashAlgorithmAsString();
        }
        if (res.second.error().code()) {
            qWarning() << "Encryption failed:" << res.second.error().asString();
            return nullptr;
        }
    } else if (sign) {
        std::unique_ptr<QGpgME::SignJob> job(proto->signJob(armor, textMode));
        auto result = job->exec(signingKeys, canonicalizeContent(content), signingMode, resultContent);
        if (result.error().code()) {
            qWarning() << "Signing failed:" << result.error().asString();
            return nullptr;
        }
        hashAlgo = result.createdSignature(0).hashAlgorithmAsString();
    } else if (encrypt) {
        std::unique_ptr<QGpgME::EncryptJob> job(proto->encryptJob(armor, textMode));
        const auto result = job->exec(encryptionKeys, canonicalizeContent(content), alwaysTrust, resultContent);
        if (result.error().code()) {
            qWarning() << "Encryption failed:" << result.error().asString();
            return nullptr;
        }
        hashAlgo = "pgp-sha1";
    } else {
        qWarning() << "Not signing or encrypting";
        return nullptr;
    }

    return composeHeadersAndBody(content, resultContent, sign, hashAlgo);
}

KMime::Content *MailCrypto::sign(KMime::Content *content, const std::vector<GpgME::Key> &signers)
{
    return processCrypto(content, signers, {}, OPENPGP);
}

std::vector<GpgME::Key> MailCrypto::findKeys(const QStringList &filter, bool findPrivate, Protocol protocol)
{
    const QGpgME::Protocol *const backend = protocol == SMIME ? QGpgME::smime() : QGpgME::openpgp();
    Q_ASSERT(backend);
    QGpgME::KeyListJob *job = backend->keyListJob(false);
    Q_ASSERT(job);

    std::vector<GpgME::Key> keys;
    GpgME::KeyListResult res = job->exec(filter, findPrivate, keys);

    Q_ASSERT(!res.error());

    qWarning() << "got keys:" << keys.size();

    for (std::vector< GpgME::Key >::iterator i = keys.begin(); i != keys.end(); ++i) {
        qWarning() << "key isnull:" << i->isNull() << "isexpired:" << i->isExpired();
        qWarning() << "key numuserIds:" << i->numUserIDs();
        for (uint k = 0; k < i->numUserIDs(); ++k) {
            qWarning() << "userIDs:" << i->userID(k).email();
        }
    }

    return keys;
}

