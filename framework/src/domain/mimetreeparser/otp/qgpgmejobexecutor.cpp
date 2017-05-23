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

#include "qgpgmejobexecutor.h"
#include "mimetreeparser_debug.h"

#include <QGpgME/DecryptVerifyJob>
#include <QGpgME/ImportJob>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/VerifyOpaqueJob>

#include <QEventLoop>

#include <cassert>

using namespace GpgME;
using namespace MimeTreeParser;

QGpgMEJobExecutor::QGpgMEJobExecutor(QObject *parent) : QObject(parent)
{
    setObjectName(QStringLiteral("KleoJobExecutor"));
    mEventLoop = new QEventLoop(this);
}

GpgME::VerificationResult QGpgMEJobExecutor::exec(
    QGpgME::VerifyDetachedJob *job,
    const QByteArray &signature,
    const QByteArray &signedData)
{
    qCDebug(MIMETREEPARSER_LOG) << "Starting detached verification job";
    connect(job, SIGNAL(result(GpgME::VerificationResult)), SLOT(verificationResult(GpgME::VerificationResult)));
    GpgME::Error err = job->start(signature, signedData);
    if (err) {
        return VerificationResult(err);
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    return mVerificationResult;
}

GpgME::VerificationResult QGpgMEJobExecutor::exec(
    QGpgME::VerifyOpaqueJob *job,
    const QByteArray &signedData,
    QByteArray &plainText)
{
    qCDebug(MIMETREEPARSER_LOG) << "Starting opaque verification job";
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

std::pair< GpgME::DecryptionResult, GpgME::VerificationResult > QGpgMEJobExecutor::exec(
    QGpgME::DecryptVerifyJob *job,
    const QByteArray &cipherText,
    QByteArray &plainText)
{
    qCDebug(MIMETREEPARSER_LOG) << "Starting decryption job";
    connect(job, &QGpgME::DecryptVerifyJob::result, this, &QGpgMEJobExecutor::decryptResult);
    GpgME::Error err = job->start(cipherText);
    if (err) {
        plainText.clear();
        return std::make_pair(DecryptionResult(err), VerificationResult(err));
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    plainText = mData;
    return std::make_pair(mDecryptResult, mVerificationResult);
}

GpgME::ImportResult QGpgMEJobExecutor::exec(QGpgME::ImportJob *job, const QByteArray &certData)
{
    connect(job, SIGNAL(result(GpgME::ImportResult)), SLOT(importResult(GpgME::ImportResult)));
    GpgME::Error err = job->start(certData);
    if (err) {
        return ImportResult(err);
    }
    mEventLoop->exec(QEventLoop::ExcludeUserInputEvents);
    return mImportResult;
}

Error QGpgMEJobExecutor::auditLogError() const
{
    return mAuditLogError;
}

void QGpgMEJobExecutor::verificationResult(const GpgME::VerificationResult &result)
{
    qCDebug(MIMETREEPARSER_LOG) << "Detached verification job finished";
    QGpgME::Job *job = qobject_cast<QGpgME::Job *>(sender());
    assert(job);
    mVerificationResult = result;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void QGpgMEJobExecutor::verificationResult(const GpgME::VerificationResult &result, const QByteArray &plainText)
{
    qCDebug(MIMETREEPARSER_LOG) << "Opaque verification job finished";
    QGpgME::Job *job = qobject_cast<QGpgME::Job *>(sender());
    assert(job);
    mVerificationResult = result;
    mData = plainText;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void QGpgMEJobExecutor::decryptResult(
    const GpgME::DecryptionResult &decryptionresult,
    const GpgME::VerificationResult &verificationresult,
    const QByteArray &plainText)
{
    qCDebug(MIMETREEPARSER_LOG) << "Decryption job finished";
    QGpgME::Job *job = qobject_cast<QGpgME::Job *>(sender());
    assert(job);
    mVerificationResult = verificationresult;
    mDecryptResult = decryptionresult;
    mData = plainText;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

void QGpgMEJobExecutor::importResult(const GpgME::ImportResult &result)
{
    QGpgME::Job *job = qobject_cast<QGpgME::Job *>(sender());
    assert(job);
    mImportResult = result;
    mAuditLogError = job->auditLogError();
    mAuditLog = job->auditLogAsHtml();
    mEventLoop->quit();
}

QString QGpgMEJobExecutor::auditLogAsHtml() const
{
    return mAuditLog;
}

