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

#ifndef DECRYPTVERIFYBODYPARTMEMENTO_H
#define DECRYPTVERIFYBODYPARTMEMENTO_H

#include "cryptobodypartmemento.h"

#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <QObject>
#include <QString>
#include <QPointer>

#include "interfaces/bodypart.h"
#include "messageviewer/viewer.h"

namespace Kleo
{
class DecryptVerifyJob;
}

namespace MessageViewer
{

class DecryptVerifyBodyPartMemento
    : public CryptoBodyPartMemento
{
    Q_OBJECT
public:
    DecryptVerifyBodyPartMemento(Kleo::DecryptVerifyJob *job, const QByteArray &cipherText);
    ~DecryptVerifyBodyPartMemento();

    bool start() Q_DECL_OVERRIDE;
    void exec() Q_DECL_OVERRIDE;

    const QByteArray &plainText() const
    {
        return m_plainText;
    }
    const GpgME::DecryptionResult &decryptResult() const
    {
        return m_dr;
    }
    const GpgME::VerificationResult &verifyResult() const
    {
        return m_vr;
    }

private Q_SLOTS:
    void slotResult(const GpgME::DecryptionResult &dr,
                    const GpgME::VerificationResult &vr,
                    const QByteArray &plainText);

private:
    void saveResult(const GpgME::DecryptionResult &,
                    const GpgME::VerificationResult &,
                    const QByteArray &);
private:
    // input:
    const QByteArray m_cipherText;
    QPointer<Kleo::DecryptVerifyJob> m_job;
    // output:
    GpgME::DecryptionResult m_dr;
    GpgME::VerificationResult m_vr;
    QByteArray m_plainText;
};
}
#endif // DECRYPTVERIFYBODYPARTMEMENTO_H
