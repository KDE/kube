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
#include "crypto.h"

#include "framework/src/errors.h"

#include <gpgme.h>

#include <QDebug>
#include <QDateTime>

#include <future>
#include <utility>

using namespace Crypto;

QDebug operator<< (QDebug d, const Key &key)
{
    d << key.fingerprint;
    return d;
}

QDebug operator<< (QDebug d, const Error &error)
{
    d << error.errorCode();
    return d;
}

struct Data {
    Data(const QByteArray &buffer)
    {
        const bool copy = false;
        const gpgme_error_t e = gpgme_data_new_from_mem(&data, buffer.constData(), buffer.size(), int(copy));
        if (e) {
            qWarning() << "Failed to copy data?" << e;
        }
    }

    ~Data()
    {
        gpgme_data_release(data);
    }
    gpgme_data_t data;
};

static gpgme_error_t checkEngine(CryptoProtocol protocol)
{
    gpgme_check_version(0);
    const gpgme_protocol_t p = protocol == CMS ? GPGME_PROTOCOL_CMS : GPGME_PROTOCOL_OpenPGP;
    return gpgme_engine_check_version(p);
}

static std::pair<gpgme_error_t, gpgme_ctx_t> createForProtocol(CryptoProtocol proto)
{
    if (auto e = checkEngine(proto)) {
        qWarning() << "GPG Engine check failed." << e;
        return std::make_pair(e, nullptr);
    }
    gpgme_ctx_t ctx = 0;
    if (auto e = gpgme_new(&ctx)) {
        return std::make_pair(e, nullptr);
    }

    switch (proto) {
        case OpenPGP:
            if (auto e = gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP)) {
                gpgme_release(ctx);
                return std::make_pair(e, nullptr);
            }
        break;
        case CMS:
            if (auto e = gpgme_set_protocol(ctx, GPGME_PROTOCOL_CMS)) {
                gpgme_release(ctx);
                return std::make_pair(e, nullptr);
            }
        break;
        default:
            Q_ASSERT(false);
            return std::make_pair(1, nullptr);
    }
    return std::make_pair(GPG_ERR_NO_ERROR, ctx);
}


struct Context {
    Context(CryptoProtocol protocol = OpenPGP)
    {
        gpgme_error_t code;
        std::tie(code, context) = createForProtocol(protocol);
        error = Error{code};
    }

    ~Context()
    {
        gpgme_release(context);
    }

    operator bool() const
    {
        return !error;
    }
    Error error;
    gpgme_ctx_t context;
};


static QByteArray toBA(gpgme_data_t out)
{
    size_t length = 0;
    auto data = gpgme_data_release_and_get_mem (out, &length);
    auto outdata = QByteArray{data, static_cast<int>(length)};
    gpgme_free(data);
    return outdata;
}

static std::vector<Recipient> copyRecipients(gpgme_decrypt_result_t result)
{
    std::vector<Recipient> recipients;
    for (gpgme_recipient_t r = result->recipients ; r ; r = r->next) {
        recipients.push_back({QByteArray{r->keyid}, {r->status}});
    }
    return recipients;
}

static std::vector<Signature> copySignatures(gpgme_verify_result_t result)
{
    std::vector<Signature> signatures;
    for (gpgme_signature_t is = result->signatures ; is ; is = is->next) {
        Signature sig;
        sig.fingerprint = QByteArray{is->fpr};
        sig.creationTime.setTime_t(is->timestamp);
        sig.summary = is->summary;
        sig.status = {is->status};
        sig.validity = is->validity;
        sig.validity_reason = is->validity_reason;
        signatures.push_back(sig);
    }
    return signatures;
}


VerificationResult Crypto::verifyDetachedSignature(CryptoProtocol protocol, const QByteArray &signature, const QByteArray &text)
{
    Context context{protocol};
    if (!context) {
        qWarning() << "Failed to create context " << context.error;
        return {{}, context.error};
    }
    auto ctx = context.context;

    auto err = gpgme_op_verify(ctx, Data{signature}.data, Data{text}.data, 0);
    gpgme_verify_result_t res = gpgme_op_verify_result(ctx);
    return {copySignatures(res), {err}};
}

VerificationResult Crypto::verifyOpaqueSignature(CryptoProtocol protocol, const QByteArray &signature, QByteArray &outdata)
{
    Context context{protocol};
    if (!context) {
        qWarning() << "Failed to create context " << context.error;
        return VerificationResult{{}, context.error};
    }
    auto ctx = context.context;

    gpgme_data_t out;
    const gpgme_error_t e = gpgme_data_new(&out);
    Q_ASSERT(!e);
    auto err = gpgme_op_verify(ctx, Data{signature}.data, 0, out);

    VerificationResult result{{}, {err}};
    if (gpgme_verify_result_t res = gpgme_op_verify_result(ctx)) {
        result.signatures = copySignatures(res);
    }

    outdata = toBA(out);
    return result;
}

std::pair<DecryptionResult,VerificationResult> Crypto::decryptAndVerify(CryptoProtocol protocol, const QByteArray &ciphertext, QByteArray &outdata)
{
    Context context{protocol};
    if (!context) {
        qWarning() << "Failed to create context " << context.error;
        return std::make_pair(DecryptionResult{{}, context.error}, VerificationResult{{}, context.error});
    }
    auto ctx = context.context;

    gpgme_data_t out;
    if (gpgme_error_t e = gpgme_data_new(&out)) {
        qWarning() << "Failed to allocated data" << e;
    }
    auto err = gpgme_op_decrypt_verify(ctx, Data{ciphertext}.data, out);
    if (err) {
        qWarning() << "Failed to decrypt and verify" << Error{err};
        //We make sure we don't return any plain-text if the decryption failed to prevent EFAIL
        if (err == GPG_ERR_DECRYPT_FAILED) {
            return std::make_pair(DecryptionResult{{}, {err}}, VerificationResult{{}, {err}});
        }
    }

    VerificationResult verificationResult{{}, {err}};
    if (gpgme_verify_result_t res = gpgme_op_verify_result(ctx)) {
        verificationResult.signatures = copySignatures(res);
    }

    DecryptionResult decryptionResult{{}, {err}};
    if (gpgme_decrypt_result_t res = gpgme_op_decrypt_result(ctx)) {
        decryptionResult.recipients = copyRecipients(res);
    }

    outdata = toBA(out);
    return std::make_pair(decryptionResult, verificationResult);
}

ImportResult Crypto::importKeys(CryptoProtocol protocol, const QByteArray &certData)
{
    Context context{protocol};
    if (!context) {
        qWarning() << "Failed to create context " << context.error;
        return {0, 0, 0};
    }
    if (auto err = gpgme_op_import(context.context, Data{certData}.data)) {
        qWarning() << "Import failed";
        return {0, 0, 0};
    }
    if (auto result = gpgme_op_import_result(context.context)) {
        return {result->considered, result->imported, result->unchanged};
    } else {
        return {0, 0, 0};
    }
}

static KeyListResult listKeys(CryptoProtocol protocol, const std::vector<const char*> &patterns, bool secretOnly, int keyListMode)
{
    Context context{protocol};
    if (!context) {
        qWarning() << "Failed to create context " << context.error;
        return {{}, context.error};
    }
    auto ctx = context.context;

    gpgme_set_keylist_mode(ctx, keyListMode);

    KeyListResult result;
    result.error = {GPG_ERR_NO_ERROR};
    if (patterns.size() > 1) {
        qWarning() << "Listing multiple patterns";
        if (auto err = gpgme_op_keylist_ext_start(ctx, const_cast<const char **>(patterns.data()), int(secretOnly), 0)) {
            qWarning() << "Error while listing keys";
            result.error = {err};
        }
    } else if (patterns.size() == 1) {
        qWarning() << "Listing one patterns " << patterns.data()[0];
        if (auto err = gpgme_op_keylist_start(ctx, patterns.data()[0], int(secretOnly))) {
            qWarning() << "Error while listing keys";
            result.error = {err};
        }
    } else {
        qWarning() << "Listing all";
        if (auto err = gpgme_op_keylist_start(ctx, 0, int(secretOnly))) {
            qWarning() << "Error while listing keys";
            result.error = {err};
        }
    }


    while (true) {
        gpgme_key_t key;
        if (auto e = gpgme_op_keylist_next(ctx, &key)) {
            break;
        }
        Key k;
        if (key->subkeys) {
            k.keyId = QByteArray{key->subkeys->keyid};
            k.shortKeyId = k.keyId.right(8);
            k.fingerprint = QByteArray{key->subkeys->fpr};
        }
        for (gpgme_user_id_t uid = key->uids ; uid ; uid = uid->next) {
            k.userIds.push_back(UserId{QByteArray{uid->name}, QByteArray{uid->email}, QByteArray{uid->uid}});
        }
        k.isExpired = key->expired;
        result.keys.push_back(k);
    }
    gpgme_op_keylist_end(ctx);
    return result;
}

/**
 * Get the given `key` in the armor format.
 */
Expected<Error, QByteArray> Crypto::exportPublicKey(const Key &key)
{
    Context context;
    if (!context) {
        return makeUnexpected(Error{context.error});
    }

    gpgme_data_t out;
    const gpgme_error_t e = gpgme_data_new(&out);
    Q_ASSERT(!e);

    qDebug() << "Exporting public key:" << key.keyId;
    if (auto err = gpgme_op_export(context.context, key.keyId, 0, out)) {
        return makeUnexpected(Error{err});
    }

    return toBA(out);
}

Expected<Error, QByteArray> Crypto::signAndEncrypt(const QByteArray &content, const std::vector<Key> &encryptionKeys, const std::vector<Key> &signingKeys)
{
    Context context;
    if (!context) {
        return makeUnexpected(Error{context.error});
    }

    for (const auto &signingKey : signingKeys) {
        //TODO do we have to free those again?
        gpgme_key_t key;
        if (auto e = gpgme_get_key(context.context, signingKey.fingerprint, &key, /*secret*/ false)) {
            qWarning() << "Failed to retrive signing key " << signingKey.fingerprint << e;
        } else {
            gpgme_signers_add(context.context, key);
        }
    }

    gpgme_key_t * const keys = new gpgme_key_t[encryptionKeys.size() + 1];
    gpgme_key_t * keys_it = keys;
    for (const auto &k : encryptionKeys) {
        gpgme_key_t key;
        if (auto e = gpgme_get_key(context.context, k.fingerprint, &key, /*secret*/ false)) {
            qWarning() << "Failed to retrive key " << k.fingerprint << e;
        } else {
            *keys_it++ = key;
        }
    }
    *keys_it++ = 0;

    gpgme_data_t out;
    const gpgme_error_t e = gpgme_data_new(&out);
    Q_ASSERT(!e);

    gpgme_error_t err = !signingKeys.empty() ?
        gpgme_op_encrypt_sign(context.context, keys, GPGME_ENCRYPT_ALWAYS_TRUST, Data{content}.data, out) :
        gpgme_op_encrypt(context.context, keys, GPGME_ENCRYPT_ALWAYS_TRUST, Data{content}.data, out);
    delete[] keys;
    if (err) {
        qWarning() << "Encryption failed:" << Error{err};
        return makeUnexpected(Error{err});
    }

    return toBA(out);
;
}

Expected<Error, std::pair<QByteArray, QString>>
Crypto::sign(const QByteArray &content, const std::vector<Key> &signingKeys)
{
    Context context;
    if (!context) {
        return makeUnexpected(Error{context.error});
    }

    for (const auto &signingKey : signingKeys) {
        //TODO do we have to free those again?
        gpgme_key_t key;
        if (auto e = gpgme_get_key(context.context, signingKey.fingerprint, &key, /*secret*/ false)) {
            qWarning() << "Failed to retrive signing key " << signingKey.fingerprint << e;
        } else {
            gpgme_signers_add(context.context, key);
        }
    }

    gpgme_data_t out;
    const gpgme_error_t e = gpgme_data_new(&out);
    Q_ASSERT(!e);

    if (auto err = gpgme_op_sign(context.context, Data{content}.data, out, GPGME_SIG_MODE_DETACH)) {
        qWarning() << "Signing failed:" << Error{err};
        return makeUnexpected(Error{err});
    }


    const QByteArray algo = [&] {
        if (gpgme_sign_result_t res = gpgme_op_sign_result(context.context)) {
            for (gpgme_new_signature_t is = res->signatures ; is ; is = is->next) {
                return QByteArray{gpgme_hash_algo_name(is->hash_algo)};
            }
        }
        return QByteArray{};
    }();
    // RFC 3156 Section 5:
    // Hash-symbols are constructed [...] by converting the text name to lower
    // case and prefixing it with the four characters "pgp-".
    const auto micAlg = (QString("pgp-") + algo).toLower();

    return std::pair<QByteArray, QString>{toBA(out), micAlg};
}

ImportResult Crypto::importKey(const QByteArray &pkey)
{
    return importKeys(OpenPGP, pkey);
}

std::vector<Key> Crypto::findKeys(const QStringList &patterns, bool findPrivate, bool remote)
{
    QByteArrayList list;
    std::transform(patterns.constBegin(), patterns.constEnd(), std::back_inserter(list), [] (const QString &s) { return s.toUtf8(); });
    std::vector<char const *> pattern;
    std::transform(list.constBegin(), list.constEnd(), std::back_inserter(pattern), [] (const QByteArray &s) { return s.constData(); });
    pattern.push_back(0);

    const KeyListResult res = listKeys(OpenPGP, pattern, findPrivate, remote ? GPGME_KEYLIST_MODE_EXTERN : GPGME_KEYLIST_MODE_LOCAL);
    if (res.error) {
        qWarning() << "Failed to lookup keys: " << res.error;
        return {};
    }
    qDebug() << "got keys:" << res.keys.size();
    for (const auto &key : res.keys) {
        qDebug() << "isexpired:" << key.isExpired;
        for (const auto &userId : key.userIds) {
            qDebug() << "userID:" << userId.email;
        }
    }
    return res.keys;
}

