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

#include "framework/src/errors.h"
#include "crypto.h"

#include <QDebug>

#include <future>
#include <utility>

using namespace MailCrypto;
using namespace Crypto;

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

/**
 * Create an Email with `msg` as a body and `key` as an attachment.
 *
 * Will create the given structure:
 *
 * + `multipart/mixed`
 *   - the given `msg`
 *   - `application/pgp-keys` (the given `key` as attachment)
 *
 * Used by the `createSignedEmail` and `createEncryptedEmail` functions.
 */
Expected<Error, std::unique_ptr<KMime::Content>>
appendPublicKey(std::unique_ptr<KMime::Content> msg, const Key &key)
{
    const auto publicKeyExportResult = exportPublicKey(key);

    if (!publicKeyExportResult) {
        // "Could not export public key"
        return makeUnexpected(Error{publicKeyExportResult.error()});
    }

    const auto publicKeyData = publicKeyExportResult.value();

    auto result = std::unique_ptr<KMime::Content>(new KMime::Content);
    result->contentType()->setMimeType("multipart/mixed");
    result->contentType()->setBoundary(KMime::multiPartBoundary());

    KMime::Content *keyAttachment = new KMime::Content;
    {
        keyAttachment->contentType()->setMimeType("application/pgp-keys");
        keyAttachment->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
        keyAttachment->contentDisposition()->setFilename(QString("0x") + key.shortKeyId + ".asc");
        keyAttachment->setBody(publicKeyData);
    }

    msg->assemble();

    result->addContent(msg.release());
    result->addContent(keyAttachment);

    result->assemble();

    return result;
}

/**
 * Create a message part like this (according to RFC 3156 Section 4):
 *
 * - multipart/encrypted
 *   - application/pgp-encrypted (version information)
 *   - application/octet-stream (given encrypted data)
 *
 * Should not be used directly since the public key should be attached, hence
 * the `createEncryptedEmail` function.
 *
 * The encrypted data can be generated by the `encrypt` or `signAndEncrypt` functions.
 */
std::unique_ptr<KMime::Content> createEncryptedPart(QByteArray encryptedData)
{
    auto result = std::unique_ptr<KMime::Content>(new KMime::Content);

    result->contentType()->setMimeType("multipart/encrypted");
    result->contentType()->setBoundary(KMime::multiPartBoundary());
    result->contentType()->setParameter("protocol", "application/pgp-encrypted");

    KMime::Content *controlInformation = new KMime::Content;
    {
        controlInformation->contentType()->setMimeType("application/pgp-encrypted");
        controlInformation->contentDescription()->from7BitString("PGP/MIME version identification");
        controlInformation->setBody("Version: 1");

        result->addContent(controlInformation);
    }

    KMime::Content *encryptedPartPart = new KMime::Content;
    {
        const QString filename = "msg.asc";

        encryptedPartPart->contentType()->setMimeType("application/octet-stream");
        encryptedPartPart->contentType()->setName(filename, "utf-8");

        encryptedPartPart->contentDescription()->from7BitString("OpenPGP encrypted message");

        encryptedPartPart->contentDisposition()->setDisposition(KMime::Headers::CDinline);
        encryptedPartPart->contentDisposition()->setFilename(filename);

        encryptedPartPart->setBody(encryptedData);

        result->addContent(encryptedPartPart);
    }

    return result;
}

/**
 * Create an encrypted (optionally signed) email with a public key attached to it.
 *
 * Will create a message like this:
 *
 * + `multipart/mixed`
 *   - `multipart/encrypted`
 *     + `application/pgp-encrypted
 *     + `application/octet-stream` (a generated encrypted version of the original message)
 *   - `application/pgp-keys` (the public key as attachment, which is the first of the
 *     `signingKeys`)
 */
Expected<Error, std::unique_ptr<KMime::Content>>
createEncryptedEmail(KMime::Content *content, const std::vector<Key> &encryptionKeys,
    const Key &attachedKey, const std::vector<Key> &signingKeys = {})
{
    auto encryptionResult = signAndEncrypt(canonicalizeContent(content), encryptionKeys, signingKeys);

    if (!encryptionResult) {
        return makeUnexpected(Error{encryptionResult.error()});
    }

    auto encryptedPart = createEncryptedPart(encryptionResult.value());

    auto publicKeyAppendResult = appendPublicKey(std::move(encryptedPart), attachedKey);

    if(publicKeyAppendResult) {
        publicKeyAppendResult.value()->assemble();
    }

    return publicKeyAppendResult;
}

/**
 * Create a message part like this (according to RFC 3156 Section 5):
 *
 * + `multipart/signed`
 *   - whatever the given original `message` is (should be canonicalized)
 *   - `application/octet-stream` (the given `signature`)
 *
 * Should not be used directly since the public key should be attached, hence
 * the `createSignedEmail` function.
 *
 * The signature can be generated by the `sign` function.
 */
std::unique_ptr<KMime::Content> createSignedPart(
    std::unique_ptr<KMime::Content> message, const QByteArray &signature, const QString &micAlg)
{
    auto result = std::unique_ptr<KMime::Content>(new KMime::Content);

    result->contentType()->setMimeType("multipart/signed");
    result->contentType()->setBoundary(KMime::multiPartBoundary());
    result->contentType()->setParameter("micalg", micAlg);
    result->contentType()->setParameter("protocol", "application/pgp-signature");

    result->addContent(message.release());

    KMime::Content *signedPartPart = new KMime::Content;
    {
        signedPartPart->contentType()->setMimeType("application/pgp-signature");
        signedPartPart->contentType()->setName("signature.asc", "utf-8");

        signedPartPart->contentDescription()->from7BitString(
            "This is a digitally signed message part");

        signedPartPart->setBody(signature);

        result->addContent(signedPartPart);
    }

    return result;
}

/**
 * Create a signed email with a public key attached to it.
 *
 * Will create a message like this:
 *
 * + `multipart/mixed`
 *   - `multipart/signed`
 *     + whatever the given original `content` is (should not be canonalized)
 *     + `application/octet-stream` (a generated signature of the original message)
 *   - `application/pgp-keys` (the public key as attachment, which is the first of the
 *     `signingKeys`)
 */
Expected<Error, std::unique_ptr<KMime::Content>>
createSignedEmail(std::unique_ptr<KMime::Content> content,
    const std::vector<Key> &signingKeys, const Key &attachedKey)
{
    Q_ASSERT(!signingKeys.empty());

    auto contentToSign = canonicalizeContent(content.get());

    auto signingResult = sign(contentToSign, signingKeys);

    if (!signingResult) {
        return makeUnexpected(Error{signingResult.error()});
    }

    QByteArray signingData;
    QString micAlg;
    std::tie(signingData, micAlg) = signingResult.value();

    auto signedPart = createSignedPart(std::move(content), signingData, micAlg);

    auto publicKeyAppendResult = appendPublicKey(std::move(signedPart), attachedKey);

    if (publicKeyAppendResult) {
        publicKeyAppendResult.value()->assemble();
    }

    return publicKeyAppendResult;
}

Expected<Error, std::unique_ptr<KMime::Content>>
MailCrypto::processCrypto(std::unique_ptr<KMime::Content> content, const std::vector<Key> &signingKeys,
    const std::vector<Key> &encryptionKeys, const Key &attachedKey)
{
    if (!encryptionKeys.empty()) {
        return createEncryptedEmail(content.release(), encryptionKeys, attachedKey, signingKeys);
    } else if (!signingKeys.empty()) {
        return createSignedEmail(std::move(content), signingKeys, signingKeys[0]);
    } else {
        qWarning() << "Processing cryptography, but neither signing nor encrypting";
        return content;
    }
}

