/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

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

#ifndef __MIMETREEPARSER_DECRYPTVERIFYBODYPARTMEMENTO_H__
#define __MIMETREEPARSER_DECRYPTVERIFYBODYPARTMEMENTO_H__

#include "cryptobodypartmemento.h"

#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <QPointer>

#include "bodypart.h"

namespace QGpgME
{
class DecryptVerifyJob;
}

namespace MimeTreeParser
{

class DecryptVerifyBodyPartMemento
    : public CryptoBodyPartMemento
{
    Q_OBJECT
public:
    DecryptVerifyBodyPartMemento(QGpgME::DecryptVerifyJob *job, const QByteArray &cipherText);
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
    QPointer<QGpgME::DecryptVerifyJob> m_job;
    // output:
    GpgME::DecryptionResult m_dr;
    GpgME::VerificationResult m_vr;
    QByteArray m_plainText;
};
}
#endif // __MIMETREEPARSER_DECRYPTVERIFYBODYPARTMEMENTO_H__
