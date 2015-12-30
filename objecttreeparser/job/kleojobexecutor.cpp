/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kleojobexecutor.h"
#include "messageviewer_debug.h"
#include <Libkleo/DecryptVerifyJob>
#include <Libkleo/ImportJob>
#include <Libkleo/VerifyDetachedJob>
#include <Libkleo/VerifyOpaqueJob>

#include <QBuffer>
#include <QEventLoop>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace MessageViewer;
using boost::shared_ptr;

KleoJobExecutor::KleoJobExecutor(QObject *parent) : QObject(parent)
{
    setObjectName(QStringLiteral("KleoJobExecutor"));
    mEventLoop = new QEventLoop(this);
}

GpgME::VerificationResult KleoJobExecutor::exec(
    Kleo::VerifyDetachedJob *job,
    const QByteArray &signature,
    const QByteArray &signedData)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Starting detached verification job";
    connect(job, SIGNAL(result(GpgME::VerificationResult)), SLOT(verificationResult(GpgME::VerificationResult)));
    GpgME::Error err = job->start(signature, signedData);
    if (err) {
        return VerificationResult(err);
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    return mVerificationResult;
}

GpgME::VerificationResult KleoJobExecutor::exec(
    Kleo::VerifyOpaqueJob *job,
    const QByteArray &signedData,
    QByteArray &plainText)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Starting opaque verification job";
    connect(job, SIGNAL(result(GpgME::VerificationResult,QByteArray)), SLOT(verificationResult(GpgME::VerificationResult,QByteArray)));
    GpgME::Error err = job->start(signedData);
    if (err) {
        plainText.clear();
        return VerificationResult(err);
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    plainText = mData;
    return mVerificationResult;
}

std::pair< GpgME::DecryptionResult, GpgME::VerificationResult > KleoJobExecutor::exec(
    Kleo::DecryptVerifyJob *job,
    const QByteArray &cipherText,
    QByteArray &plainText)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Starting decryption job";
    connect(job, &DecryptVerifyJob::result, this, &KleoJobExecutor::decryptResult);
    GpgME::Error err = job->start(cipherText);
    if (err) {
        plainText.clear();
        return std::make_pair(DecryptionResult(err), VerificationResult(err));
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    plainText = mData;
    return std::make_pair(mDecryptResult, mVerificationResult);
}

GpgME::ImportResult KleoJobExecutor::exec(Kleo::ImportJob *job, const QByteArray &certData)
{
    connect(job, SIGNAL(result(GpgME::ImportResult)), SLOT(importResult(GpgME::ImportResult)));
    GpgME::Error err = job->start(certData);
    if (err) {
        return ImportResult(err);
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    return mImportResult;
}

Error KleoJobExecutor::auditLogError() const
{
    return mAuditLogError;
}

void KleoJobExecutor::verificationResult(const GpgME::VerificationResult &result)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Detached verification job finished";
    Kleo::Job *job = dynamic_cast<Kleo::Job *>(sender());
    assert(job);
    mVerificationResult = result;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void KleoJobExecutor::verificationResult(const GpgME::VerificationResult &result, const QByteArray &plainText)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Opaque verification job finished";
    Kleo::Job *job = dynamic_cast<Kleo::Job *>(sender());
    assert(job);
    mVerificationResult = result;
    mData = plainText;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void KleoJobExecutor::decryptResult(
    const GpgME::DecryptionResult &decryptionresult,
    const GpgME::VerificationResult &verificationresult,
    const QByteArray &plainText)
{
    qCDebug(MESSAGEVIEWER_LOG) << "Decryption job finished";
    Kleo::Job *job = dynamic_cast<Kleo::Job *>(sender());
    assert(job);
    mVerificationResult = verificationresult;
    mDecryptResult = decryptionresult;
    mData = plainText;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void KleoJobExecutor::importResult(const GpgME::ImportResult &result)
{
    Kleo::Job *job = dynamic_cast<Kleo::Job *>(sender());
    assert(job);
    mImportResult = result;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

QString KleoJobExecutor::auditLogAsHtml() const
{
    return mAuditLog;
}

