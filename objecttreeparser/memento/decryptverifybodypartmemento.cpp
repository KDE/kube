/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "decryptverifybodypartmemento.h"

#include <Libkleo/DecryptVerifyJob>

#include <qstringlist.h>

using namespace Kleo;
using namespace GpgME;
using namespace MessageViewer;

DecryptVerifyBodyPartMemento::DecryptVerifyBodyPartMemento(DecryptVerifyJob *job, const QByteArray &cipherText)
    : CryptoBodyPartMemento(),
      m_cipherText(cipherText),
      m_job(job)
{
    assert(m_job);
}

DecryptVerifyBodyPartMemento::~DecryptVerifyBodyPartMemento()
{
    if (m_job) {
        m_job->slotCancel();
    }
}

bool DecryptVerifyBodyPartMemento::start()
{
    assert(m_job);
    if (const Error err = m_job->start(m_cipherText)) {
        m_dr = DecryptionResult(err);
        return false;
    }
    connect(m_job.data(), &DecryptVerifyJob::result,
            this, &DecryptVerifyBodyPartMemento::slotResult);
    setRunning(true);
    return true;
}

void DecryptVerifyBodyPartMemento::exec()
{
    assert(m_job);
    QByteArray plainText;
    setRunning(true);
    const std::pair<DecryptionResult, VerificationResult> p = m_job->exec(m_cipherText, plainText);
    saveResult(p.first, p.second, plainText);
    m_job->deleteLater(); // exec'ed jobs don't delete themselves
    m_job = 0;
}

void DecryptVerifyBodyPartMemento::saveResult(const DecryptionResult &dr,
        const VerificationResult &vr,
        const QByteArray &plainText)
{
    assert(m_job);
    setRunning(false);
    m_dr = dr;
    m_vr = vr;
    m_plainText = plainText;
    setAuditLog(m_job->auditLogError(), m_job->auditLogAsHtml());
}

void DecryptVerifyBodyPartMemento::slotResult(const DecryptionResult &dr,
        const VerificationResult &vr,
        const QByteArray &plainText)
{
    saveResult(dr, vr, plainText);
    m_job = 0;
    notify();
}
