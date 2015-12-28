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

#ifndef VERIFYOPAQUEBODYPARTMEMENTO_H
#define VERIFYOPAQUEBODYPARTMEMENTO_H

#include "cryptobodypartmemento.h"
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/key.h>

#include <QObject>
#include <QString>
#include <QPointer>

#include "interfaces/bodypart.h"
#include "messageviewer/viewer.h"

namespace Kleo
{
class VerifyOpaqueJob;
class KeyListJob;
}

class QStringList;

namespace MessageViewer
{

class VerifyOpaqueBodyPartMemento
    : public CryptoBodyPartMemento
{
    Q_OBJECT
public:
    VerifyOpaqueBodyPartMemento(Kleo::VerifyOpaqueJob *job,
                                Kleo::KeyListJob *klj,
                                const QByteArray &signature);
    ~VerifyOpaqueBodyPartMemento();

    bool start() Q_DECL_OVERRIDE;
    void exec() Q_DECL_OVERRIDE;

    const QByteArray &plainText() const
    {
        return m_plainText;
    }
    const GpgME::VerificationResult &verifyResult() const
    {
        return m_vr;
    }
    const GpgME::Key &signingKey() const
    {
        return m_key;
    }

private Q_SLOTS:
    void slotResult(const GpgME::VerificationResult &vr,
                    const QByteArray &plainText);
    void slotKeyListJobDone();
    void slotNextKey(const GpgME::Key &);

private:
    void saveResult(const GpgME::VerificationResult &,
                    const QByteArray &);
    bool canStartKeyListJob() const;
    QStringList keyListPattern() const;
    bool startKeyListJob();
private:
    // input:
    const QByteArray m_signature;
    QPointer<Kleo::VerifyOpaqueJob> m_job;
    QPointer<Kleo::KeyListJob> m_keylistjob;
    // output:
    GpgME::VerificationResult m_vr;
    QByteArray m_plainText;
    GpgME::Key m_key;
};

}

#endif // VERIFYOPAQUEBODYPARTMEMENTO_H
