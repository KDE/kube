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

#include "verifydetachedbodypartmemento.h"
#include "messageviewer_debug.h"

#include <Libkleo/VerifyDetachedJob>
#include <Libkleo/KeyListJob>

#include <gpgme++/keylistresult.h>

#include <qstringlist.h>

#include <cassert>

using namespace Kleo;
using namespace GpgME;
using namespace MessageViewer;

VerifyDetachedBodyPartMemento::VerifyDetachedBodyPartMemento(VerifyDetachedJob *job,
        KeyListJob *klj,
        const QByteArray &signature,
        const QByteArray &plainText)
    : CryptoBodyPartMemento(),
      m_signature(signature),
      m_plainText(plainText),
      m_job(job),
      m_keylistjob(klj)
{
    assert(m_job);
}

VerifyDetachedBodyPartMemento::~VerifyDetachedBodyPartMemento()
{
    if (m_job) {
        m_job->slotCancel();
    }
    if (m_keylistjob) {
        m_keylistjob->slotCancel();
    }
}

bool VerifyDetachedBodyPartMemento::start()
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento started";
#endif
    connect(m_job, SIGNAL(result(GpgME::VerificationResult)),
            this, SLOT(slotResult(GpgME::VerificationResult)));
    if (const Error err = m_job->start(m_signature, m_plainText)) {
        m_vr = VerificationResult(err);
#ifdef DEBUG_SIGNATURE
        qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento stopped with error";
#endif
        return false;
    }
    setRunning(true);
    return true;
}

void VerifyDetachedBodyPartMemento::exec()
{
    assert(m_job);
    setRunning(true);
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento execed";
#endif
    saveResult(m_job->exec(m_signature, m_plainText));
    m_job->deleteLater(); // exec'ed jobs don't delete themselves
    m_job = 0;
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento after execed";
#endif
    if (canStartKeyListJob()) {
        std::vector<GpgME::Key> keys;
        m_keylistjob->exec(keyListPattern(), /*secretOnly=*/false, keys);
        if (!keys.empty()) {
            m_key = keys.back();
        }
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();    // exec'ed jobs don't delete themselves
    }
    m_keylistjob = 0;
    setRunning(false);
}

bool VerifyDetachedBodyPartMemento::canStartKeyListJob() const
{
    if (!m_keylistjob) {
        return false;
    }
    const char *const fpr = m_vr.signature(0).fingerprint();
    return fpr && *fpr;
}

QStringList VerifyDetachedBodyPartMemento::keyListPattern() const
{
    assert(canStartKeyListJob());
    return QStringList(QString::fromLatin1(m_vr.signature(0).fingerprint()));
}

void VerifyDetachedBodyPartMemento::saveResult(const VerificationResult &vr)
{
    assert(m_job);
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento::saveResult called";
#endif
    m_vr = vr;
    setAuditLog(m_job->auditLogError(), m_job->auditLogAsHtml());
}

void VerifyDetachedBodyPartMemento::slotResult(const VerificationResult &vr)
{
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento::slotResult called";
#endif
    saveResult(vr);
    m_job = 0;
    if (canStartKeyListJob() && startKeyListJob()) {
#ifdef DEBUG_SIGNATURE
        qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento: canStartKeyListJob && startKeyListJob";
#endif
        return;
    }
    if (m_keylistjob) {
        m_keylistjob->deleteLater();
    }
    m_keylistjob = 0;
    setRunning(false);
    notify();
}

bool VerifyDetachedBodyPartMemento::startKeyListJob()
{
    assert(canStartKeyListJob());
    if (const GpgME::Error err = m_keylistjob->start(keyListPattern())) {
        return false;
    }
    connect(m_keylistjob, SIGNAL(done()), this, SLOT(slotKeyListJobDone()));
    connect(m_keylistjob, SIGNAL(nextKey(GpgME::Key)),
            this, SLOT(slotNextKey(GpgME::Key)));
    return true;
}

void VerifyDetachedBodyPartMemento::slotNextKey(const GpgME::Key &key)
{
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento::slotNextKey called";
#endif
    m_key = key;
}

void VerifyDetachedBodyPartMemento::slotKeyListJobDone()
{
#ifdef DEBUG_SIGNATURE
    qCDebug(MESSAGEVIEWER_LOG) << "tokoe: VerifyDetachedBodyPartMemento::slotKeyListJobDone called";
#endif
    m_keylistjob = 0;
    setRunning(false);
    notify();
}
